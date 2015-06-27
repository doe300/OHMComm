/* 
 * File:   OHMComm.cpp
 * Author: daniel, jonas
 *
 * Created on March 29, 2015, 1:49 PM
 */

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>


#include "UserInput.h"

//dependencies
#include "RtAudio.h"
#include "AudioHandlerFactory.h"
#include "UDPWrapper.h"
#include "ProcessorRTP.h"
#include "RTPListener.h"
#include "ProcessorOpus.h"

//Declare Configurations
NetworkConfiguration networkConfiguration;


using namespace std;

// method for printing vectors used in configureAudioDevices
void printVector(std::vector<unsigned int> v)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		std::cout << v[i] << " ";
	}
}

/*!
 * Lets the user choose a supported audio-format.
 * RtAudio guarantees all formats to be supported, not only the native ones
 * 
 */
RtAudioFormat selectAudioFormat(RtAudioFormat nativeFormats)
{
    vector<string> formatNames;
    vector<RtAudioFormat> audioFormats;
    //preallocate vectors
    formatNames.reserve(8);
    audioFormats.reserve(8);
    //fill options
    formatNames.push_back(string("8 bit signed integer")+ ((nativeFormats & RTAUDIO_SINT8) == RTAUDIO_SINT8 ? " (native)" : ""));
    audioFormats.push_back(RTAUDIO_SINT8);
    formatNames.push_back(string("16 bit signed integer") + ((nativeFormats & RTAUDIO_SINT16) == RTAUDIO_SINT16 ? " (native)" : ""));
    audioFormats.push_back(RTAUDIO_SINT16);
    formatNames.push_back(string("24 bit signed integer") + ((nativeFormats & RTAUDIO_SINT24) == RTAUDIO_SINT24 ? " (native)" : ""));
    audioFormats.push_back(RTAUDIO_SINT24);
    formatNames.push_back(string("32 bit signed integer") + ((nativeFormats & RTAUDIO_SINT32) == RTAUDIO_SINT32 ? " (native)" : ""));
    audioFormats.push_back(RTAUDIO_SINT32);
    formatNames.push_back(string("32 bit float (normalized between +/- 1)") + ((nativeFormats & RTAUDIO_FLOAT32) == RTAUDIO_FLOAT32 ? " (native)" : ""));
    audioFormats.push_back(RTAUDIO_FLOAT32);
    formatNames.push_back(string("64 bit float (normalized between +/- 1)") + ((nativeFormats & RTAUDIO_FLOAT64) == RTAUDIO_FLOAT64 ? " (native)" : ""));
    audioFormats.push_back(RTAUDIO_FLOAT64);
    int formatIndex = selectOptionIndex("Choose audio format", formatNames, 0);
    //return selected option
    return audioFormats[formatIndex];
}

void configureNetwork()
{    
    cout << "Network configuration" << endl;
    
    //1. remote address
    string ipString = inputString("1. Input destination IP address");
    networkConfiguration.addressOutgoing = ipString;
    
    //2. remote and local ports
    int destPort = inputNumber("2. Input destination port", false, false);
    int localPort = inputNumber("3. Input local port", false, false);
    networkConfiguration.portOutgoing = destPort;
	networkConfiguration.portIncoming = localPort;

    cout << "Networkconfiguration set." << endl;
}

AudioConfiguration configureAudioDevices()
{
    AudioConfiguration audioConfiguration;
    cout << endl;
    cout << "+++Audio Device configuration+++" << endl;
    cout << endl;

    RtAudio AudioDevices;
    // Determine the number of available Audio Devices 
    unsigned int availableAudioDevices = AudioDevices.getDeviceCount();

    cout << "Available Audio Devices: " << endl;
    cout << endl;
    // printing available Audio Devices
    RtAudio::DeviceInfo DeviceInfo;

    unsigned int DefaultOutputDeviceID;
    unsigned int DefaultInputDeviceID;

    for (unsigned int i = 0 ; i < availableAudioDevices ; i++)
    {
            DeviceInfo = AudioDevices.getDeviceInfo(i);
            if (DeviceInfo.probed == true) //Audio Device successfully probed
            {
                    cout << "Device ID: " << i << endl;
                    cout << "Device Name = " << DeviceInfo.name << endl;
                    cout << "Maximum output channels = " << DeviceInfo.outputChannels << endl;
                    cout << "Maximum input channels = " << DeviceInfo.inputChannels << endl;
                    cout << "Maximum duplex channels = " << DeviceInfo.duplexChannels << endl;
                    cout << "Default output: ";
                    if (DeviceInfo.isDefaultOutput == true)
                    {
                            cout << "Yes" << endl;
                            DefaultOutputDeviceID = i;

                    }
                    else
                    {
                            cout << "No" << endl;
                    }
                    cout << "Default input: ";
                    if (DeviceInfo.isDefaultInput == true)
                    {
                            cout << "Yes" << endl;
                            DefaultInputDeviceID = i;
                    }
                    else
                    {
                            cout << "No" << endl;
                    }
                    cout << "Supported Sample Rates: ";
                    printVector(DeviceInfo.sampleRates);
                    cout << endl;
                    cout << "Audio Format = " << DeviceInfo.nativeFormats << endl;
                    cout << endl;
            }
    }

    unsigned int OutputDeviceID;

    //Choose output Device
    cout << "Choose output Device ID [default is " << DefaultOutputDeviceID << " ]: ";
    cin >> OutputDeviceID;

    RtAudio::DeviceInfo OutputDeviceInfo = AudioDevices.getDeviceInfo(OutputDeviceID);

    //Configure ID of the Output Audio Device
    audioConfiguration.outputDeviceID = OutputDeviceID;
    cout << "-> Using output Device ID: " << audioConfiguration.outputDeviceID << endl;

    //Configure Name of the Output Audio Device
    audioConfiguration.outputDeviceName = OutputDeviceInfo.name;
    cout << "-> Using output Device Name: " << audioConfiguration.outputDeviceName << endl;

    //Configure Number of Maximum output Channels (we always use stereo)
    audioConfiguration.outputDeviceChannels = 2;
    cout << "-> Number of maximum duplex Channels supported from this Device: " << audioConfiguration.outputDeviceChannels << endl;

    unsigned int OutputSampleRate;

    //Configure Output Sample Rate
    cout << "-> Supported Sample Rates from this Device: ";
    printVector(OutputDeviceInfo.sampleRates);
    cout << endl << "Choose your Sample Rate: ";
    cin >> OutputSampleRate;
    audioConfiguration.sampleRate = OutputSampleRate;
    cout << "-> Using Sample Rate: " << audioConfiguration.sampleRate << endl;

    //Configure Output Audio Format
    audioConfiguration.audioFormat = selectAudioFormat(OutputDeviceInfo.nativeFormats);
    cout << "-> Output Audio Format: " << audioConfiguration.audioFormat << endl;

    unsigned int InputDeviceID;

    //Choose input Device
    cout << "Choose input Device ID [default is " << DefaultInputDeviceID << " ]: ";
    cin >> InputDeviceID;

    RtAudio::DeviceInfo InputDeviceInfo = AudioDevices.getDeviceInfo(InputDeviceID);

    //Configure ID of the Input Audio Device
    audioConfiguration.inputDeviceID = InputDeviceID;
    cout << "-> Using input Device ID " << audioConfiguration.inputDeviceID << endl;

    //Configure Name of the Input Audio Device
    audioConfiguration.inputDeviceName = InputDeviceInfo.name;
    cout << "-> Using input Device Name: " << audioConfiguration.inputDeviceName << endl;

    //Configure Number of Maximum output Channels (we always use stereo)
    audioConfiguration.inputDeviceChannels = 2;
    cout << "-> Number of maximum duplex Channels supported from this Device: " << audioConfiguration.inputDeviceChannels << endl;

    unsigned int InputSampleRate;

    //TODO sampleRate and audioFormat are overridden!
    //Configure Input Sample Rate
    cout << "-> Supported Sample Rates from this Device: ";
    printVector(InputDeviceInfo.sampleRates);
    cout << endl << "Choose your Sample Rate: ";
    cin >> InputSampleRate;
    audioConfiguration.sampleRate = InputSampleRate;
    cout << "-> Using Sample Rate: " << audioConfiguration.sampleRate << endl;

    //Configure Input Audio Format
    audioConfiguration.audioFormat = selectAudioFormat(InputDeviceInfo.nativeFormats);
    cout << "-> Input Audio Format: " << audioConfiguration.audioFormat << endl;
        
    //Buffer size
    audioConfiguration.bufferFrames = inputNumber("Input the number of frames to buffer (around 128 - 2048, 256 is recommended)", false, false);
    
    return audioConfiguration;
}

/*!
 * RtAudio Error Handler
 */
void errorHandler( RtAudioError::Type type, const std::string &errorText )
{
    cerr << "An error occurred!" << endl;
    switch(type)
    {
        case RtAudioError::Type::WARNING:
        case RtAudioError::Type::DEBUG_WARNING:
            cerr << "WARNING: ";
        case RtAudioError::Type::MEMORY_ERROR:
        case RtAudioError::Type::DRIVER_ERROR:
        case RtAudioError::Type::SYSTEM_ERROR:
        case RtAudioError::Type::THREAD_ERROR:
            cerr << "ERROR: ";
        case RtAudioError::Type::NO_DEVICES_FOUND:
        case RtAudioError::Type::INVALID_DEVICE:
            cerr << "Device Error: ";
        case RtAudioError::Type::INVALID_PARAMETER:
        case RtAudioError::Type::INVALID_USE:
            cerr << "Invalid Function-call: ";
        case RtAudioError::Type::UNSPECIFIED:
            cerr << "Unspecified Error: ";
    }
    cerr << errorText << endl;
}



int main(int argc, char** argv)
{
    //Vectors for storing available AudioHandlers, and -Processors
    const std::vector<std::string> audioHandlers = {"RTAudioWrapper"};
    const std::vector<std::string> audioProcessors = {};
    
    char input;
    
    /* Run Tests */
    try
    {
        std::unique_ptr<AudioHandler> audioObject;

        ////
        //Audio-Config
        ////

        cout << "Load default audio config? Yes (y), No (n)?" << endl;
        cin >> input;

        if (input == 'N' || input == 'n')
        {
            //manually configure audio-handler, so manually select it
            std::string selectedAudioHander;
            if(audioHandlers.size() > 1)
            {
                //only let user choose if there is more than 1 audio-handler
                selectedAudioHander = selectOption("Select audio handler", audioHandlers, "RTAudioWrapper");
            }
            else 
            {
                selectedAudioHander = *audioHandlers.begin();
            }
            std::cout << "Using AudioHandler: " << selectedAudioHander << std::endl;
            AudioConfiguration audioConfiguration = configureAudioDevices();
            audioObject = AudioHandlerFactory::getAudioHandler(selectedAudioHander, audioConfiguration);
        }
        else
        {
            //use RTAudioWrapper as default implementation
            audioObject = AudioHandlerFactory::getAudioHandler("RTAudioWrapper");
        }

        ////
        // Network-Config
        ////

        NetworkWrapper *network;
        cout << "Load default network config? Yes (y), No (n)?" << endl;
        cin >> input;

        if (input == 'N' || input == 'n')
        {
                configureNetwork();
                network = new UDPWrapper(networkConfiguration);
        }
        else
        {
                networkConfiguration.addressOutgoing = "127.0.0.1";
                //the port should be a number greater than 1024
                networkConfiguration.portIncoming = 12345;
                networkConfiguration.portOutgoing = 12345;
                network = new UDPWrapper(networkConfiguration);
        }

		// TODO select and add processors

        ////
        // Startup
        ////

        //initialize RTPBuffer and -Listener
        std::unique_ptr<RTPBuffer> *rtpBuffer = new std::unique_ptr<RTPBuffer>(new RTPBuffer(256, 1000));
        
        // add a processor to the process chain
        ProcessorRTP rtp("RTP-Processor", network, rtpBuffer);
        audioObject->addProcessor(&rtp);

        //configure all processors
        audioObject->prepare();

        RTPListener listener(network, rtpBuffer, audioObject->getBufferSize());
        // start audio processing
        listener.startUp();
        audioObject->startDuplexMode();

        // wait for exit
        cout << "Type Enter to exit" << endl;
        cin >> input;


        audioObject->stop();
        listener.shutdown();
        //TODO fix error on shutdown
        return 0;
    }
    catch (RtAudioError exception)
    {
        cout << "Exception: " << exception.getMessage() << endl;
    }
}


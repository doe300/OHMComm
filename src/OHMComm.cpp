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
#include "Parameters.h"

//dependencies
#include "RtAudio.h"
#include "AudioHandlerFactory.h"
#include "UDPWrapper.h"
#include "ProcessorRTP.h"
#include "RTPListener.h"
#include "AudioProcessorFactory.h"
#include "Statistics.h"

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
    int formatIndex = UserInput::selectOptionIndex("Choose audio format", formatNames, 0);
    //return selected option
    return audioFormats[formatIndex];
}

NetworkConfiguration configureNetwork()
{    
    NetworkConfiguration networkConfiguration{0};
    UserInput::printSection("Network configuration");
    
    //1. remote address
    string ipString = UserInput::inputString("1. Input destination IP address");
    networkConfiguration.addressOutgoing = ipString;
    
    //2. remote and local ports
    int destPort = UserInput::inputNumber("2. Input destination port", false, false);
    int localPort = UserInput::inputNumber("3. Input local port", false, false);
    networkConfiguration.portOutgoing = destPort;
    networkConfiguration.portIncoming = localPort;

    cout << "Network configuration set." << endl;
    
    return networkConfiguration;
}

AudioConfiguration configureAudioDevices()
{
    AudioConfiguration audioConfiguration = { 0 };
    UserInput::printSection("Audio configuration");

    RtAudio AudioDevices;
    // Determine the number of available Audio Devices 
    unsigned int availableAudioDevices = AudioDevices.getDeviceCount();
    vector<string> audioDeviceNames(availableAudioDevices);

    cout << "Available Audio Devices: " << endl;
    cout << endl;
    // printing available Audio Devices
    RtAudio::DeviceInfo DeviceInfo;

    unsigned int DefaultOutputDeviceID = 0;
    unsigned int DefaultInputDeviceID = 0;

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
            cout << "Native Audio Formats Flag = " << DeviceInfo.nativeFormats << endl;
            cout << endl;
            
            audioDeviceNames[i] = (DeviceInfo.name + (DeviceInfo.isDefaultInput && DeviceInfo.isDefaultOutput ? " (default in/out)" : 
                (DeviceInfo.isDefaultInput ? " (default in)" : (DeviceInfo.isDefaultOutput ? " (default out)" : ""))));
        }
    }

    //Choose output Device
    unsigned int OutputDeviceID = UserInput::selectOptionIndex("Choose output Device ID", audioDeviceNames, DefaultOutputDeviceID);

    RtAudio::DeviceInfo OutputDeviceInfo = AudioDevices.getDeviceInfo(OutputDeviceID);

    //Configure ID of the Output Audio Device
    audioConfiguration.outputDeviceID = OutputDeviceID;
    cout << "-> Using output Device ID: " << audioConfiguration.outputDeviceID << endl;

    //Configure Name of the Output Audio Device
    audioConfiguration.outputDeviceName = OutputDeviceInfo.name;
    cout << "-> Using output Device Name: " << audioConfiguration.outputDeviceName << endl;

    //Configure outputDeviceChannels (we always use stereo)
    audioConfiguration.outputDeviceChannels = 2;

    //sample-rate, audio-format and buffer-size is defined by the enabled Processors so we can skip them

    //Choose input Device
    unsigned int InputDeviceID = UserInput::selectOptionIndex("Choose input Device ID", audioDeviceNames, DefaultInputDeviceID);

    RtAudio::DeviceInfo InputDeviceInfo = AudioDevices.getDeviceInfo(InputDeviceID);

    //Configure ID of the Input Audio Device
    audioConfiguration.inputDeviceID = InputDeviceID;
    cout << "-> Using input Device ID " << audioConfiguration.inputDeviceID << endl;

    //Configure Name of the Input Audio Device
    audioConfiguration.inputDeviceName = InputDeviceInfo.name;
    cout << "-> Using input Device Name: " << audioConfiguration.inputDeviceName << endl;

    //Configure inputDeviceChannels (we always use stereo)
    audioConfiguration.inputDeviceChannels = 2;

    return audioConfiguration;
}

AudioConfiguration fillAudioConfiguration(int outputDeviceID, int inputDeviceID)
{
    AudioConfiguration config = { 0 };
    RtAudio audioDevices;
    if(outputDeviceID < 0)
    {
        outputDeviceID = audioDevices.getDefaultOutputDevice();
    }
    if(inputDeviceID < 0)
    {
        inputDeviceID = audioDevices.getDefaultInputDevice();
    }
    //we always use stereo
    config.outputDeviceChannels = 2;
    config.inputDeviceChannels = 2;
    
    config.outputDeviceID = outputDeviceID;
    config.outputDeviceName = audioDevices.getDeviceInfo(outputDeviceID).name;
    
    config.inputDeviceID = inputDeviceID;
    config.inputDeviceName = audioDevices.getDeviceInfo(inputDeviceID).name;
    return config;
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


int main(int argc, char* argv[])
{
    Parameters params(AudioProcessorFactory::getAudioProcessorNames());
    bool runWithArguments = params.parseParameters(argc, argv);
    
    //Vectors for storing available AudioHandlers, and -Processors
    const std::vector<std::string> audioHandlers = AudioHandlerFactory::getAudioHandlerNames();
    std::vector<std::string> audioProcessors = AudioProcessorFactory::getAudioProcessorNames();
    
    char input;
    
    try
    {
        std::unique_ptr<AudioHandler> audioObject;

        ////
        //Audio-Config
        ////
        if(runWithArguments)
        {
            //get device configuration from parameters
            int outputDeviceID = -1;
            int inputDeviceID = -1;
            if(params.isParameterSet(Parameters::OUTPUT_DEVICE))
            {
                outputDeviceID = atoi(params.getParameterValue(Parameters::OUTPUT_DEVICE).c_str());
            }
            if(params.isParameterSet(Parameters::INPUT_DEVICE))
            {
                inputDeviceID = atoi(params.getParameterValue(Parameters::INPUT_DEVICE).c_str());
            }
            AudioConfiguration audioConfig = fillAudioConfiguration(outputDeviceID, inputDeviceID);
            audioObject = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::getDefaultAudioHandlerName(), audioConfig);
        }
        else
        {
            cout << "Load default audio config? Yes (y), No (n)?" << endl;
            cin >> input;

            if (input == 'N' || input == 'n')
            {
                //manually configure audio-handler, so manually select it
                std::string selectedAudioHander;
                if(audioHandlers.size() > 1)
                {
                    //only let user choose if there is more than 1 audio-handler
                    selectedAudioHander = UserInput::selectOption("Select audio handler", audioHandlers, AudioHandlerFactory::getDefaultAudioHandlerName());
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
                //use RtAudioWrapper as default implementation
                audioObject = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::getDefaultAudioHandlerName());
            }
        }

        ////
        // Network-Config
        ////

        std::shared_ptr<NetworkWrapper> network;
        if(runWithArguments)
        {
            NetworkConfiguration networkConfiguration{0};
            networkConfiguration.addressOutgoing = params.getParameterValue(Parameters::REMOTE_ADDRESS);
            networkConfiguration.portIncoming = atoi(params.getParameterValue(Parameters::LOCAL_PORT).c_str());
            networkConfiguration.portOutgoing = atoi(params.getParameterValue(Parameters::REMOTE_PORT).c_str());
            std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(networkConfiguration));
            network = std::move(tmp);
        }
        else
        {
             cout << "Load default network config? Yes (y), No (n)?" << endl;
            cin >> input;

            if (input == 'N' || input == 'n')
            {
                NetworkConfiguration networkConfiguration = configureNetwork();
                std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(networkConfiguration));
                network = std::move(tmp);    
            }
            else
            {
                NetworkConfiguration networkConfiguration{0};
                networkConfiguration.addressOutgoing = "127.0.0.1";
                //the port should be a number greater than 1024
                networkConfiguration.portIncoming = DEFAULT_NETWORK_PORT;
                networkConfiguration.portOutgoing = DEFAULT_NETWORK_PORT;
                std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(networkConfiguration));
                network = std::move(tmp);
            }
        }

	////
        // AudioProcessors
        ////
        bool createProfiler;
        bool logStatisticsToFile;
        std::string statisticsLogFile;
        if(runWithArguments)
        {
            createProfiler = params.isParameterSet(Parameters::PROFILE_PROCESSORS);
            for(const std::string procName : params.getAudioProcessors())
            {
                AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(procName, createProfiler);
                if(proc != nullptr)
                {
                    audioObject->addProcessor(proc);
                }
            }
            logStatisticsToFile = params.isParameterSet(Parameters::LOG_TO_FILE);
            statisticsLogFile = params.getParameterValue(Parameters::LOG_TO_FILE);
        }
        else
        {
            createProfiler = UserInput::inputBoolean("Create profiler for audio-processors?");
            logStatisticsToFile = UserInput::inputBoolean("Log statistics and profiling-results to file?");
            if(logStatisticsToFile)
            {
                statisticsLogFile = UserInput::inputString("Type log-file name");
            }
            // add processors to the process chain
            audioProcessors.push_back("End");
            unsigned int selectedIndex;
            std::cout << "The AudioProcessors should be added in the order they are used on the sending side!" << std::endl;
            while((selectedIndex = UserInput::selectOptionIndex("Select next AudioProcessor to add", audioProcessors, audioProcessors.size()-1)) != audioProcessors.size()-1)
            {
                audioObject->addProcessor(AudioProcessorFactory::getAudioProcessor(audioProcessors.at(selectedIndex), createProfiler));
                audioProcessors.at(selectedIndex) = audioProcessors.at(selectedIndex) + " (added)";
            }
        }

        ////
        // Startup
        ////

        //initialize RTPBuffer and -Listener
        std::shared_ptr<RTPBuffer> rtpBuffer(new RTPBuffer(256, 1000));

        ProcessorRTP* rtp = new ProcessorRTP("RTP-Processor", network, rtpBuffer);
        if(createProfiler)
        {
            //enabled profiling of the RTP processor
            ProfilingAudioProcessor* rtpProfiler = new ProfilingAudioProcessor(rtp);
            audioObject->addProcessor(rtpProfiler);
        }
        else
        {
            audioObject->addProcessor(rtp);
        }


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
        network->closeNetwork();
        
        if(logStatisticsToFile)
        {
            Statistics::printStatisticsToFile(statisticsLogFile);
        }
        //print statistics to stdout anyway
        Statistics::printStatistics();
        return 0;
    }
    catch (RtAudioError exception)
    {
        cout << "Exception: " << exception.getMessage() << endl;
    }
}


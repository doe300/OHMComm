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
#include "configuration.h"

//dependencies for rtaudio
#include "../lib/rtaudio-4.1.1/RtAudio.h"
#include "UDPWrapper.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h> // sockaddr_in
#endif


//Declare Configurations
NetworkConfiguration networkConfiguration;
AudioConfiguration audioConfiguration;
AudioProcessor *topOfChain;

using namespace std;

//conversion method
//returns -1 on error
inline int convertToInt(const std::string& s)
{
    std::istringstream i(s);
    int x;
    if (!(i >> x))
        return -1;
    return x;
}

// method for printing vectors used in configureAudioDevices
void printVector(std::vector<unsigned int> v)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		std::cout << v[i] << " ";
	}
}

/*!
 * Lets the user choose a supported audio-format
 */
RtAudioFormat selectAudioFormat(RtAudioFormat supportedFormats)
{
    cout << "Choose audio format" << endl;
    if((supportedFormats & RTAUDIO_SINT8) == RTAUDIO_SINT8)
    {
        cout << RTAUDIO_SINT8+0 << ": 8 bit signed integer" << endl;
    }
    if((supportedFormats & RTAUDIO_SINT16) == RTAUDIO_SINT16)
    {
        cout << RTAUDIO_SINT16+0 << ": 16 bit signed integer" << endl;
    }
    if((supportedFormats & RTAUDIO_SINT24) == RTAUDIO_SINT24)
    {
        cout << RTAUDIO_SINT24+0 << ": 24 bit signed integer" << endl;
    }
    if((supportedFormats & RTAUDIO_SINT32) == RTAUDIO_SINT32)
    {
        cout << RTAUDIO_SINT32+0 << ": 32 bit signed integer" << endl;
    }
    if((supportedFormats & RTAUDIO_FLOAT32) == RTAUDIO_FLOAT32)
    {
        cout << RTAUDIO_FLOAT32+0 << ": 32 bit float (normalized between +/- 1)" << endl;
    }
    if((supportedFormats & RTAUDIO_FLOAT64) == RTAUDIO_FLOAT64)
    {
        cout << RTAUDIO_FLOAT64+0 << ": 64 bit float (normalized between +/- 1)" << endl;
    }
    RtAudioFormat format;
    cin >> format;
    return format;
}

inline void createAddress(sockaddr *addr, int addressType, std::string ipString, int port)
{
    sockaddr_in *address = reinterpret_cast<sockaddr_in*>(addr);
    //IPv4 or IPv6
    address->sin_family = addressType;
    
    if(ipString == "")
    {
        //listen on any address
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        //the remote IP address - inet_addr() is a convenience-method to convert from aaa.bbb.ccc.ddd (octal representation) to a 4-byte unsigned long
        address->sin_addr.s_addr = inet_addr(ipString.c_str());
    }
    //network byte order is big-endian, so we must convert the port-number
    address->sin_port = htons(port);
}

void configureNetwork()
{
    string ipString;
    string destPortString, localPortString;
    string protocolString;
    
    cout << "Network configuration:" << endl;
    
    //1. remote address
    cout << "1. Input destination IP address: ";
    cin >> ipString;
    
    //2. remote and local ports
    cout << "2. Input destination port: ";
    cin >> destPortString;
    cout << "3. Input local port: ";
    cin >> localPortString;
    
    //3. protocol
    cout << "4. Choose protocol (TCP/UDP) [defaults to UDP]: ";
    cin >> protocolString;
    
    //4. parse arguments
    int destPort = convertToInt(destPortString);
    int localPort = convertToInt(localPortString);
    
    if(protocolString == "TCP")
    {
        // SOCK_STREAM - creating a stream-socket
        // IPPROTO_TCP - use TCP/IP protocol
        networkConfiguration.socketType = SOCK_STREAM;
        networkConfiguration.protocol = IPPROTO_TCP;
        cout << "-> Using TCP (stream-socket)" << endl;
    }
    else
    {
        // SOCK_DGRAM - creating a datagram-socket
        // IPPROTO_UDP - use UDP protocol
        networkConfiguration.socketType = SOCK_DGRAM;
        networkConfiguration.protocol = IPPROTO_UDP;
        cout << "-> Using UDP (datagram-socket)" << endl;
    }
    
    //5. create addresses
    //local address
    createAddress(&networkConfiguration.localAddr, AF_INET, "", localPort);
    //remote address
    createAddress(&networkConfiguration.remoteAddr, AF_INET, ipString, destPort);
    
    cout << "Network configured!" << endl;
    cout << endl;
}

void configureAudioDevices()
{
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
	audioConfiguration.OutputDeviceID = OutputDeviceID;
	cout << "-> Using output Device ID: " << audioConfiguration.OutputDeviceID << endl;

	//Configure Name of the Output Audio Device
	audioConfiguration.OutputDeviceName = OutputDeviceInfo.name;
	cout << "-> Using output Device Name: " << audioConfiguration.OutputDeviceName << endl;

	//Configure Number of Maximum output Channels
	audioConfiguration.OutputDeviceChannels = OutputDeviceInfo.outputChannels;
	cout << "-> Number of maximum output Channels supported from this Device: " << audioConfiguration.OutputDeviceChannels << endl;

	unsigned int OutputSampleRate;

	//Configure Output Sample Rate
	cout << "-> Supported Sample Rates from this Device: ";
	printVector(OutputDeviceInfo.sampleRates);
	cout << endl << "Choose your Sample Rate: ";
	cin >> OutputSampleRate;
	audioConfiguration.OutputSampleRate = OutputSampleRate;
	cout << "-> Using Sample Rate: " << audioConfiguration.OutputSampleRate << endl;

	//Configure Output Audio Format
	audioConfiguration.OutputAudioFormat = selectAudioFormat(OutputDeviceInfo.nativeFormats);
	cout << "-> Output Audio Format: " << audioConfiguration.OutputAudioFormat << endl;

	unsigned int InputDeviceID;

	//Choose input Device
	cout << "Choose input Device ID [default is " << DefaultInputDeviceID << " ]: ";
	cin >> InputDeviceID;

	RtAudio::DeviceInfo InputDeviceInfo = AudioDevices.getDeviceInfo(InputDeviceID);

	//Configure ID of the Input Audio Device
	audioConfiguration.InputDeviceID = InputDeviceID;
	cout << "-> Using input Device ID " << audioConfiguration.InputDeviceID << endl;

	//Configure Name of the Input Audio Device
	audioConfiguration.InputDeviceName = InputDeviceInfo.name;
	cout << "-> Using input Device Name: " << audioConfiguration.InputDeviceName << endl;

	//Configure Number of Maximum output Channels
	audioConfiguration.InputDeviceChannels = InputDeviceInfo.inputChannels;
	cout << "-> Number of maximum input Channels supported from this Device: " << audioConfiguration.InputDeviceChannels << endl;

	unsigned int InputSampleRate;

	//Configure Input Sample Rate
	cout << "-> Supported Sample Rates from this Device: ";
	printVector(InputDeviceInfo.sampleRates);
	cout << endl << "Choose your Sample Rate: ";
	cin >> InputSampleRate;
	audioConfiguration.InputSampleRate = InputSampleRate;
	cout << "-> Using Sample Rate: " << audioConfiguration.InputSampleRate << endl;

	//Configure Input Audio Format
	audioConfiguration.InputAudioFormat = selectAudioFormat(InputDeviceInfo.nativeFormats);
	cout << "-> Input Audio Format: " << audioConfiguration.InputAudioFormat << endl;
}

uint8_t addAudioProcessor(std::string names[], uint8_t numberOfProcessors, uint8_t alreadyAdded[])
{
    cout << "Choose an AudioProcessor to add: " << endl;
    for(uint8_t i= 0; i<numberOfProcessors ; i++)
    {
        cout << +i << ": ";
        cout << names[i];
        if(alreadyAdded[i] != 0)
        {
            cout << " (added)";
        }
        cout << endl;
    }
    cout << +numberOfProcessors << ": finish" << endl;
    unsigned int index;
    cin >> index;
    return index;
}

AudioProcessor *addAudioProcessor(std::string processorName, AudioProcessor *underlying)
{
    //TODO map name to processor
    if(processorName == "UDPWrapper")
    {
        return new UDPWrapper();
    }
    //fall back to previous processor
    return underlying;
}

AudioProcessor *selectAudioProcessors()
{
    AudioProcessor *processor = NULL;
    const uint8_t numberOfProcessors = 1;
    std::string names[] = {std::string("UDPWrapper")};
    uint8_t alreadyAdded[numberOfProcessors] = {0};
    //1. add (ordered) audio-processors
    cout << endl;
    cout << "Select the AudioProcessors to add, beginning from the bottom of the chain (I/O)" << endl;
    uint8_t processorIndex;
    while((processorIndex = addAudioProcessor(names, numberOfProcessors, alreadyAdded)) < numberOfProcessors)
    {
        cout << "Adding " << names[processorIndex] << endl;
        processor = addAudioProcessor(names[processorIndex], processor);
        alreadyAdded[processorIndex] = 1;
    }
    //2. return the top audio processor
    return processor;
}

/*!
 * Returns, whether the default config was loaded
 */
int loadDefaultConfig()
{
    cout << "Load the default configuration?[Y/N]" << endl;
    std::string loadAnswer;
    cin >> loadAnswer;
    if(loadAnswer == "Y" || loadAnswer== "Yes" || loadAnswer == "y" || loadAnswer == "yes")
    {
        //network configuration - local port on loopback device
        createAddress(&networkConfiguration.localAddr, AF_INET, "", 54321);
        createAddress(&networkConfiguration.remoteAddr, AF_INET, "", 54321);
        networkConfiguration.socketType = SOCK_DGRAM;
        networkConfiguration.protocol = IPPROTO_UDP;
        
        //audio configuration - use default devices and sample-rate
        RtAudio audioDevices;
        unsigned int defaultInputDeviceID = audioDevices.getDefaultInputDevice();
        unsigned int defaultOutputDeviceID = audioDevices.getDefaultOutputDevice();
        
        //input device
        RtAudio::DeviceInfo inputDeviceInfo = audioDevices.getDeviceInfo(defaultInputDeviceID);
        audioConfiguration.InputDeviceID = defaultInputDeviceID;
        audioConfiguration.InputDeviceName = inputDeviceInfo.name;
        audioConfiguration.InputDeviceChannels = inputDeviceInfo.inputChannels;
        audioConfiguration.InputSampleRate = 44100; //TODO check device support
        //choose the most exact audio-format supported by the device
        audioConfiguration.InputAudioFormat = inputDeviceInfo.nativeFormats & RTAUDIO_FLOAT64 || 
                inputDeviceInfo.nativeFormats & RTAUDIO_FLOAT32 || inputDeviceInfo.nativeFormats & RTAUDIO_SINT32 ||
                inputDeviceInfo.nativeFormats & RTAUDIO_SINT24 || inputDeviceInfo.nativeFormats & RTAUDIO_SINT16 ||
                inputDeviceInfo.nativeFormats & RTAUDIO_SINT8;
        
        //output device
        RtAudio::DeviceInfo outputDeviceInfo = audioDevices.getDeviceInfo(defaultOutputDeviceID);
        audioConfiguration.OutputDeviceID = defaultOutputDeviceID;
        audioConfiguration.OutputDeviceName = outputDeviceInfo.name;
        audioConfiguration.OutputDeviceChannels = outputDeviceInfo.inputChannels;
        audioConfiguration.OutputSampleRate = 44100; //TODO check device support
        //choose the most exact audio-format supported by the device
        audioConfiguration.OutputAudioFormat = outputDeviceInfo.nativeFormats & RTAUDIO_FLOAT64 || 
                outputDeviceInfo.nativeFormats & RTAUDIO_FLOAT32 || outputDeviceInfo.nativeFormats & RTAUDIO_SINT32 ||
                outputDeviceInfo.nativeFormats & RTAUDIO_SINT24 || outputDeviceInfo.nativeFormats & RTAUDIO_SINT16 ||
                outputDeviceInfo.nativeFormats & RTAUDIO_SINT8;
        
        cout << "Default config loaded" << endl;
        return 1;
    }
    return 0;
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

/*!
 * RtAudio Callback
 */
int audioCallback( void *outputBuffer, void *inputBuffer,unsigned int nFrames,double streamTime,
                                RtAudioStreamStatus status, void *userData )
{
    //need to wrap the AudioProcessor, because of the hidden this-parameter of c++ instance methods
    return topOfChain->process(outputBuffer, inputBuffer, nFrames, streamTime, status, userData);
}

int main(int argc, char** argv)
{
    ////
    // Configuration
    ////

    //0. check for default config
    if(!loadDefaultConfig())
    {
        //1. network connection
        configureNetwork();

        //2. audio devices
        configureAudioDevices();
    }
    
    //3. processors
    topOfChain= selectAudioProcessors();
    
    ////
    // Initialize
    ////
    
    //1. RTP
    //2. AudioProcessors
    topOfChain->configure();
    //3. RTAudio
    //number of frames buffered - TODO configure
    unsigned int bufferFrames = 32;
    
    RtAudio audio;
    RtAudio::StreamParameters inputParams;
    inputParams.deviceId = audioConfiguration.InputDeviceID;
    inputParams.nChannels = audioConfiguration.InputDeviceChannels;
    RtAudio::StreamParameters outputParams;
    outputParams.deviceId = audioConfiguration.OutputDeviceID;
    outputParams.nChannels = audioConfiguration.OutputDeviceChannels;
    
    audio.openStream(&outputParams, &inputParams, 
                     (RtAudioFormat)audioConfiguration.InputAudioFormat, 
                     audioConfiguration.InputSampleRate, 
                     &bufferFrames, &audioCallback, NULL,
                     NULL, &errorHandler);
    ////
    // Running
    ////
    
    //start loop
    audio.startStream();

    char input;
    std::cout << "\nRunning ... press <enter> to quit (buffer frames = " << bufferFrames << ").\n";
    cin >> input;

    // Stop the stream.
    audio.stopStream();
    
    return 0;
}


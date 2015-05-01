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
#include "UserInput.h"

//dependencies for rtaudio
#include "RtAudio.h"
#include "UDPWrapper.h"
#include "FileProcessor.h"

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
    vector<string> formatNames;
    vector<RtAudioFormat> audioFormats;
    //preallocate vectors
    formatNames.reserve(8);
    audioFormats.reserve(8);
    //fill options
    if((supportedFormats & RTAUDIO_SINT8) == RTAUDIO_SINT8)
    {
        formatNames.push_back("8 bit signed integer");
        audioFormats.push_back(RTAUDIO_SINT8);
    }
    if((supportedFormats & RTAUDIO_SINT16) == RTAUDIO_SINT16)
    {
        formatNames.push_back("16 bit signed integer");
        audioFormats.push_back(RTAUDIO_SINT16);
    }
    if((supportedFormats & RTAUDIO_SINT24) == RTAUDIO_SINT24)
    {
        formatNames.push_back("24 bit signed integer");
        audioFormats.push_back(RTAUDIO_SINT24);
    }
    if((supportedFormats & RTAUDIO_SINT32) == RTAUDIO_SINT32)
    {
        formatNames.push_back("32 bit signed integer");
        audioFormats.push_back(RTAUDIO_SINT32);
    }
    if((supportedFormats & RTAUDIO_FLOAT32) == RTAUDIO_FLOAT32)
    {
        formatNames.push_back("32 bit float (normalized between +/- 1)");
        audioFormats.push_back(RTAUDIO_FLOAT32);
    }
    if((supportedFormats & RTAUDIO_FLOAT64) == RTAUDIO_FLOAT64)
    {
        formatNames.push_back("64 bit float (normalized between +/- 1)");
        audioFormats.push_back(RTAUDIO_FLOAT64);
    }
    int formatIndex = selectOptionIndex("Choose audio format", formatNames, 0);
    //return selected option
    return audioFormats[formatIndex];
}

/*!
 * Automatically selects the best audio format out of the supported formats
 */
RtAudioFormat autoSelectAudioFormat(RtAudioFormat supportedFormats)
{
    if((supportedFormats & RTAUDIO_FLOAT64) == RTAUDIO_FLOAT64)
    {
        return RTAUDIO_FLOAT64;
    }
    if((supportedFormats & RTAUDIO_FLOAT32) == RTAUDIO_FLOAT32)
    {
        return RTAUDIO_FLOAT32;
    }
    if((supportedFormats & RTAUDIO_SINT32) == RTAUDIO_SINT32)
    {
        return RTAUDIO_SINT32;
    }
    if((supportedFormats & RTAUDIO_SINT24) == RTAUDIO_SINT24)
    {
        return RTAUDIO_SINT24;
    }
    if((supportedFormats & RTAUDIO_SINT16) == RTAUDIO_SINT16)
    {
        return RTAUDIO_SINT16;
    }
    //fall back to worst quality
    return RTAUDIO_SINT8;
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
    string protocolString;
    
    cout << "Network configuration" << endl;
    
    //1. remote address
    string ipString = inputString("1. Input destination IP address");
    
    //2. remote and local ports
    int destPort = inputNumber("2. Input destination port", false, false);
    int localPort = inputNumber("3. Input local port", false, false);
    
    //3. protocol
//    cout << "4. Choose protocol (TCP/UDP) [defaults to UDP]";
//    cin >> protocolString;
    vector<string> protocols {"TCP", "UDP"};
    protocolString = selectOption("4. Choose protocol", protocols, "UDP");
    
    //4. parse arguments
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

AudioProcessor *addAudioProcessor(std::string processorName, AudioProcessor *previousProcessor)
{
    AudioProcessor *processor = NULL;
    //TODO map name to processor
    if(processorName == "UDPWrapper")
    {
        processor = new UDPWrapper();
    }
	else if (processorName == "FileProcessor")
	{
		processor = new FileProcessor("recording.rtaudio", false, true);
	}
    if(processor == NULL)
    {
        //TODO, throw error
    }
    if(previousProcessor != NULL)
    {
        previousProcessor->setNextInChain(processor);
    }
    return processor;
}

void selectAudioProcessors()
{
    AudioProcessor *processor = NULL;
    const uint8_t numberOfProcessors = 2;
    //last "processor" is the finish to exit from loop
    vector<string> names {"UDPWrapper", "FileProcessor", "finish"};
    uint8_t alreadyAdded[numberOfProcessors] = {0};
    //1. add (ordered) audio-processors
    cout << endl;
    cout << "Select the AudioProcessors to add, in the order of their execution" << endl;
    uint8_t processorIndex;
    while((processorIndex = selectOptionIndex("Choose an AudioProcessor to add", names, names.size()-1)) < numberOfProcessors)
    {
        cout << "Adding: " << names[processorIndex] << endl;
        processor = addAudioProcessor(names[processorIndex], processor);
        if(topOfChain == NULL)
        {
            //assign the first created processor as top-of-chain
            topOfChain = processor;
        }
        //add "(added)" mark to processor-name, but only once
        if(alreadyAdded[processorIndex] == 0)
        {
            names[processorIndex] = names[processorIndex] + " (added)";
        }
        alreadyAdded[processorIndex] = 1;
    }
}

/*!
 * Returns, whether the default config was loaded
 */
int loadDefaultConfig()
{
    if(inputBoolean("Load the default configuration?"))
    {
        //network configuration - local port on loopback device
        createAddress(&networkConfiguration.localAddr, AF_INET, "127.0.0.1", 54321);
        createAddress(&networkConfiguration.remoteAddr, AF_INET, "127.0.0.1", 54321);
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
        audioConfiguration.InputAudioFormat = autoSelectAudioFormat(inputDeviceInfo.nativeFormats);
        
        //output device
        RtAudio::DeviceInfo outputDeviceInfo = audioDevices.getDeviceInfo(defaultOutputDeviceID);
        audioConfiguration.OutputDeviceID = defaultOutputDeviceID;
        audioConfiguration.OutputDeviceName = outputDeviceInfo.name;
        audioConfiguration.OutputDeviceChannels = outputDeviceInfo.outputChannels;
        audioConfiguration.OutputSampleRate = 44100; //TODO check device support
        //choose the most exact audio-format supported by the device
        audioConfiguration.OutputAudioFormat = autoSelectAudioFormat(outputDeviceInfo.nativeFormats);
        
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

	try
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
		selectAudioProcessors();
    
		////
		// Initialize
		////
    
		//1. RTP
		//2. AudioProcessors
                AudioProcessor *nextInChain = topOfChain;
                while(nextInChain != NULL)
                {
                    nextInChain->configure();
                    nextInChain = nextInChain->getNextInChain();
                }
		//3. RTAudio
		//number of frames buffered - TODO configure
		unsigned int bufferFrames = 256;
    
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
	catch (RtAudioError exception)
	{
		cout << "Ausnahme: " << exception.getMessage() << endl;
	}
}


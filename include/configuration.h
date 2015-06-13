/* 
 * File:   configuration.h
 * Author: daniel, jonas
 *
 * Created on April 1, 2015, 5:37 PM
 */

#ifndef CONFIGURATION_H
#define	CONFIGURATION_H

#include <string>

struct NetworkConfiguration {
	enum ConnectionType { TCP = 1, UDP = 2 };
        //Local IP address
	std::string addressIncoming;
        //Local port
	unsigned short portIncoming;

        //Remote IP address
	std::string addressOutgoing;
        //Remote port
	unsigned short portOutgoing;

	unsigned int outputBufferSize;
	unsigned int inputBufferSize;

	ConnectionType connectionType;
};

struct AudioConfiguration {
	// These parameters are needed for the RtAudio::StreamParameter (Struct)

	// Output Audio Device ID
	unsigned int outputDeviceID;
	// input audio device ID
	unsigned int inputDeviceID;

	// number of maximum output channels supported by the output device
	unsigned int outputDeviceChannels;
	// number of maximum input channels supported by the input device
	unsigned int inputDeviceChannels;

	// number of maximum output channels supported by the output device
	unsigned int outputDeviceFirstChannel;
	// number of maximum input channels supported by the input device
	unsigned int inputDeviceFirstChannel;


	// the name of the output audio device (optional)
	std::string outputDeviceName;

	// the name of the output audio device (optional)
	std::string inputDeviceName;

	// RtAudioformat, which is defined as: typedef unsigned long RtAudioFormat;
	unsigned long audioFormat;

	// sample rate of the audio device
	unsigned int sampleRate;

	// buffer frames
	unsigned int bufferFrames;
};

//Configurations are declared in OhmComm.cpp
//this two lines are required for all files to find the configurations
extern NetworkConfiguration networkConfiguration;
extern AudioConfiguration audioConfiguration;
#endif	/* CONFIGURATION_H */


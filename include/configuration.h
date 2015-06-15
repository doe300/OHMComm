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

	friend bool operator==(const AudioConfiguration& lhs, const AudioConfiguration& rhs)
	{
		if (lhs.outputDeviceID != rhs.outputDeviceID)
			return false;
		if (lhs.inputDeviceID != rhs.inputDeviceID)
			return false;
		if (lhs.outputDeviceChannels != rhs.outputDeviceChannels)
			return false;
		if (lhs.inputDeviceChannels != rhs.inputDeviceChannels)
			return false;
		if (lhs.outputDeviceFirstChannel != rhs.outputDeviceFirstChannel)
			return false;
		if (lhs.inputDeviceFirstChannel != rhs.inputDeviceFirstChannel)
			return false;
		if (lhs.outputDeviceName != rhs.outputDeviceName)
			return false;
		if (lhs.inputDeviceName != rhs.inputDeviceName)
			return false;
		if (lhs.audioFormat != rhs.audioFormat)
			return false;
		if (lhs.sampleRate != rhs.sampleRate)
			return false;
		if (lhs.bufferFrames != rhs.bufferFrames)
			return false;

		return true;
	}
};

//Configurations are declared in OhmComm.cpp
//this two lines are required for all files to find the configurations
extern NetworkConfiguration networkConfiguration;
extern AudioConfiguration audioConfiguration;
#endif	/* CONFIGURATION_H */


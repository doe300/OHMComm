/* 
 * File:   configuration.h
 * Author: daniel, jonas
 *
 * Created on April 1, 2015, 5:37 PM
 */

#ifndef CONFIGURATION_H
#define	CONFIGURATION_H

#include <string>

struct NetworkConfiguration
{
    //we don't need local IP, because we listen on any address
    //Local port
    unsigned short portIncoming;

    //Remote IP address
    std::string addressOutgoing;
    //Remote port
    unsigned short portOutgoing;
};

struct AudioConfiguration
{
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
    
    //The AUDIO_FORMAT_XXX flags are the same as the RtAudioFormats
    static const unsigned int AUDIO_FORMAT_SINT8 = 0x1;    // 8-bit signed integer.
    static const unsigned int AUDIO_FORMAT_SINT16 = 0x2;   // 16-bit signed integer.
    static const unsigned int AUDIO_FORMAT_SINT24 = 0x4;   // 24-bit signed integer.
    static const unsigned int AUDIO_FORMAT_SINT32 = 0x8;   // 32-bit signed integer.
    static const unsigned int AUDIO_FORMAT_FLOAT32 = 0x10; // Normalized between plus/minus 1.0.
    static const unsigned int AUDIO_FORMAT_FLOAT64 = 0x20; // Normalized between plus/minus 1.0.
    static const unsigned int AUDIO_FORMAT_ALL = 0xFFFFFFFF; // supports all audio-formats
    
    static const unsigned int SAMPLE_RATE_8000 = 1;         // 8 kHz
    static const unsigned int SAMPLE_RATE_12000 = 2;        // 12 kHz
    static const unsigned int SAMPLE_RATE_16000 = 4;        // 16 kHz
    static const unsigned int SAMPLE_RATE_24000 = 8;        // 24 kHz
    static const unsigned int SAMPLE_RATE_32000 = 16;       // 32 kHz
    static const unsigned int SAMPLE_RATE_44100 = 32;       // 44.1 kHz
    static const unsigned int SAMPLE_RATE_48000 = 64;       // 48 kHz
    static const unsigned int SAMPLE_RATE_96000 = 128;      // 96 kHz
    static const unsigned int SAMPLE_RATE_192000 = 256;     // 192 kHz
    static const unsigned int SAMPLE_RATE_ALL = 0xFFFFFFFF; // supports all sample-rates
};

//Configurations are declared in OhmComm.cpp
//this two lines are required for all files to find the configurations
extern NetworkConfiguration networkConfiguration;
#endif	/* CONFIGURATION_H */


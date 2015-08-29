/*
 * File:   configuration.h
 * Author: daniel, jonas
 *
 * Created on April 1, 2015, 5:37 PM
 */

#ifndef CONFIGURATION_H
#define	CONFIGURATION_H

#define TESTMODE 0
#define PACKET_LOSS_CHANCE_IN_PERCENT 0

#if TESTMODE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#endif

#include <string>

//the port should be a number greater than 1024
static const int DEFAULT_NETWORK_PORT = 12345;
//The program's current version as string
static const std::string OHMCOMM_VERSION = "0.3";

struct NetworkConfiguration
{
    //we don't need local IP, because we listen on any address (of the local device)
    //Local port
    unsigned short localPort;
    //Remote IP address
    std::string remoteIPAddress;
    //Remote port
    unsigned short remotePort;
};

struct AudioConfiguration
{
    // the library-specific ID of the audio output-device to use
    unsigned int outputDeviceID;
    // the library-specific ID of the input-device
    unsigned int inputDeviceID;

    // number of maximum output channels supported by the output device
    unsigned int outputDeviceChannels;
    // number of maximum input channels supported by the input device
    unsigned int inputDeviceChannels;

    /*!
     * flag for the used audio-format as specified in the AudioConfiguration::AUDIO_FORMAT_XXX-flags
     * 
     * Unless the forceAudioFormatFlag is set, the audio-format (and the sample-rate) is determined by the AudioHandler 
     * by matching the supported audio-formats of the audio-processors
     */ 
    unsigned long audioFormatFlag;

    // sample rate of the audio device
    unsigned int sampleRate;

    // number of buffer frames to be sent in a single package
    unsigned int framesPerPackage;
    
    // set if configuration forces a specific sample-rate
    unsigned int forceSampleRate;
    
    // set if configuration forces a specific audio-format
    unsigned int forceAudioFormatFlag;
    
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
        if (lhs.audioFormatFlag != rhs.audioFormatFlag)
            return false;
        if (lhs.sampleRate != rhs.sampleRate)
            return false;
        if (lhs.framesPerPackage != rhs.framesPerPackage)
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
    
    /*!
     * Returns a human-readable string for the given AUDIO_FORMAT_XXX flag
     * 
     * \param audioFormatFlag AUDIO_FORMAT_XXX-flag
     * 
     * \param longDescription Whether to return a longer descriptions containing use-cases for the given audio-format
     */
    static const std::string getAudioFormatDescription(const unsigned int audioFormatFlag, bool longDescription)
    {
        switch(audioFormatFlag)
        {
            case AudioConfiguration::AUDIO_FORMAT_SINT8: return "8-bit signed integer";
            case AudioConfiguration::AUDIO_FORMAT_SINT16: 
                return !longDescription ? "16-bit signed integer" : "16-bit signed integer (default for PCM samples, required for WAV, supported by Opus)";
            case AudioConfiguration::AUDIO_FORMAT_SINT24: return "24-bit signed integer";
            case AudioConfiguration::AUDIO_FORMAT_SINT32: return "32-bit signed integer";
            case AudioConfiguration::AUDIO_FORMAT_FLOAT32: 
                return !longDescription ? "32-bit float" : "32-bit float (supported by Opus)";
            case AudioConfiguration::AUDIO_FORMAT_FLOAT64: return "64-bit float";
            default: return "unrecognized audio-format";
        }
    }
};

#endif	/* CONFIGURATION_H */


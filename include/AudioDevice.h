/* 
 * File:   AudioDevice.h
 * Author: daniel
 *
 * Created on February 22, 2016, 11:36 AM
 */

#ifndef AUDIODEVICE_H
#define	AUDIODEVICE_H

#include <string>
#include <vector>

//Container for storing info about a single audio-device, library-independent

struct AudioDevice
{
    //name of this audio-device
    const std::string name;
    //the maximum number of available output-channels
    //a value of 0 signals a non-output device
    const unsigned int outputChannels;
    //the maximum number of available input-channels
    //a value of 0 signals a non-input device
    const unsigned int inputChannels;
    //whether this device is the default output device
    const bool defaultOutputDevice;
    //whether this device is the default input device
    const bool defaultInputDevice;
    //a bit-mask of natively supported audio-formats
    const unsigned int nativeFormats;
    //whether this audio-device supports arbitrary audio-formats, otherwise, only the #nativeFormats are supported
    const bool supportsArbitraryFormats;
    //a list of all supported sample-rates
    const std::vector<unsigned int> sampleRates;

    inline bool isOutputDevice() const
    {
        return outputChannels > 0;
    }

    inline bool isInputDevice() const
    {
        return inputChannels > 0;
    }
};

#endif	/* AUDIODEVICE_H */


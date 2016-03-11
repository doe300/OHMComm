/* 
 * File:   ProcessorALaw.cpp
 * Author: daniel
 * 
 * Created on January 7, 2016, 10:56 AM
 */

#include "codecs/ProcessorALaw.h"
#include <string.h>

static constexpr ProcessorCapabilities g711Capabilities = {true, false, false, false, false, 0, 0};

ProcessorALaw::ProcessorALaw(const std::string& name) : AudioProcessor(name, g711Capabilities)
{
}

ProcessorALaw::~ProcessorALaw()
{
    delete[] writeBuffer;
}

unsigned int ProcessorALaw::getSupportedAudioFormats() const
{
    //on audio-handler side we use 16bit integer, which will be compressed into 8 bit
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int ProcessorALaw::getSupportedSampleRates() const
{
    //ITU-T G.711 says, it should be 8kHz, but the algorithm itself doesn't care
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

const std::vector<int> ProcessorALaw::getSupportedBufferSizes(unsigned int sampleRate) const
{
    //as of RFC3551 we use a default package size of 20ms -> 160 frames at 8kHz sampling rate
    const int defaultPackageSize = 20 * sampleRate / 1000;    //20 ms
    return {defaultPackageSize, BUFFER_SIZE_ANY};
}

PayloadType ProcessorALaw::getSupportedPlayloadType() const
{
    return PayloadType::PCMA;
}

void ProcessorALaw::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    maxBufferSize = audioConfig.framesPerPackage * audioConfig.outputDeviceChannels;
    writeBuffer = new int16_t[maxBufferSize];
}

bool ProcessorALaw::cleanUp()
{
    delete[] writeBuffer;
    //set buffer to nullptr to prevent duplicate free on calling destructor
    writeBuffer = nullptr;
    return true;
}

unsigned int ProcessorALaw::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    const int16_t* readBuffer = (int16_t*) inputBuffer;
    uint8_t* writeBuffer = (uint8_t*) inputBuffer;
    //2 Bytes per input-sample -> right shift to divide by 2
    const unsigned int numSamples = inputBufferByteSize >> 1;
    for(unsigned int i = 0; i < numSamples;i++)
    {
        //this works, since the output type is smaller than the input type, so we don't override unread data
        *(writeBuffer + i) = s16_to_alaw(*(readBuffer + i));
    }
    //after conversion, we have numSamples * 1 Byte
    return numSamples;
}

unsigned int ProcessorALaw::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    //WARNING: we cant just copy from and to the same buffer, since the output type is larger than the input
    //and would override our next input samples!!
    const uint8_t* readBuffer = (uint8_t*) outputBuffer;
    //number of samples == number of bytes
    //for some unknown reason, we sometimes receive a package with more than double the number of bytes, but never send such peak
    //could just have been a problem with one of the audio-devices on my computer, but to be sure, I retain calculating the minimum
    const unsigned int bufferSize = maxBufferSize > outputBufferByteSize ? outputBufferByteSize : maxBufferSize;
    for(unsigned int i = 0; i < bufferSize;i++)
    {
        //decompress samples in local buffer
        *(writeBuffer + i) = alaw_to_s16(*(readBuffer + i));
    }
    //we have now 2 Bytes per sample
    memcpy(outputBuffer, writeBuffer, bufferSize << 1);
    return bufferSize << 1;
}
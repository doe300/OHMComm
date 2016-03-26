/* 
 * File:   G711Mulaw.cpp
 * Author: daniel
 * 
 * Created on January 8, 2016, 11:38 AM
 */

#include "codecs/G711Mulaw.h"
#include <string.h>

static constexpr ohmcomm::ProcessorCapabilities g711Capabilities = {true, false, false, false, false, 0, 0};

using namespace ohmcomm::codecs;

G711Mulaw::G711Mulaw(const std::string& name) : AudioProcessor(name, g711Capabilities)
{
}

G711Mulaw::~G711Mulaw()
{
    delete[] writeBuffer;
}

unsigned int G711Mulaw::getSupportedAudioFormats() const
{
    //on audio-handler side we use 16bit integer, which will be compressed into 8 bit
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int G711Mulaw::getSupportedSampleRates() const
{
    //ITU-T G.711 says, it should be 8kHz, but the algorithm itself doesn't care
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

const std::vector<int> G711Mulaw::getSupportedBufferSizes(unsigned int sampleRate) const
{
    //as of RFC3551 we use a default package size of 20ms -> 160 frames at 8kHz sampling rate
    const int defaultPackageSize = 20 * sampleRate / 1000;    //20 ms
    return {defaultPackageSize, BUFFER_SIZE_ANY};
}

ohmcomm::PayloadType G711Mulaw::getSupportedPlayloadType() const
{
    return PayloadType::PCMU;
}

void G711Mulaw::configure(const ohmcomm::AudioConfiguration& audioConfig, const std::shared_ptr<ohmcomm::ConfigurationMode> configMode, const uint16_t bufferSize, const ProcessorCapabilities& chainCapabilities)
{
    maxBufferSize = audioConfig.framesPerPackage * audioConfig.outputDeviceChannels;
    writeBuffer = new int16_t[maxBufferSize];
}

bool G711Mulaw::cleanUp()
{
    delete[] writeBuffer;
    //set buffer to nullptr to prevent duplicate free on calling destructor
    writeBuffer = nullptr;
    return true;
}

unsigned int G711Mulaw::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, ohmcomm::StreamData* userData)
{
    const int16_t* readBuffer = (int16_t*) inputBuffer;
    uint8_t* writeBuffer = (uint8_t*) inputBuffer;
    //2 Bytes per input-sample -> right shift to divide by 2
    const unsigned int numSamples = inputBufferByteSize >> 1;
    for(unsigned int i = 0; i < numSamples;i++)
    {
        //this works, since the output type is smaller than the input type, so we don't override unread data
        *(writeBuffer + i) = s16_to_ulaw(*(readBuffer + i));
    }
    //after conversion, we have numSamples * 1 Byte
    return numSamples;
}

unsigned int G711Mulaw::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, ohmcomm::StreamData* userData)
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
        *(writeBuffer + i) = ulaw_to_s16(*(readBuffer + i));
    }
    //we have now 2 Bytes per sample
    memcpy(outputBuffer, writeBuffer, bufferSize << 1);
    return bufferSize << 1;
}
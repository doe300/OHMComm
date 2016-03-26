/* 
 * File:   Resampler.cpp
 * Author: daniel
 * 
 * Created on March 2, 2016, 4:58 PM
 */

#include <iostream>
#include <string.h>

#include "processors/Resampler.h"

using namespace ohmcomm;

Resampler::Resampler(const std::string& name, const std::vector<unsigned int>& inputSampleRates) : AudioProcessor(name), 
        inputSampleRates(inputSampleRates), writeBuffer(nullptr)
{
}

Resampler::~Resampler()
{
    if(writeBuffer != nullptr)
        delete[] writeBuffer;
}

unsigned int Resampler::getSupportedAudioFormats() const
{
    //we support every format except 24 bit signed integer, because there is no such native data type
    //since we don't know the size (in bytes) of float and double, we need to check these here
    return AudioConfiguration::AUDIO_FORMAT_SINT8 | AudioConfiguration::AUDIO_FORMAT_SINT16 | AudioConfiguration::AUDIO_FORMAT_SINT32 | 
            (sizeof(float) == 4 ? AudioConfiguration::AUDIO_FORMAT_FLOAT32 : 0) | (sizeof(double) == 8 ? AudioConfiguration::AUDIO_FORMAT_FLOAT64 : 0);
}

void Resampler::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize, const ProcessorCapabilities& chainCapabilities)
{
    numInputChannels = audioConfig.inputDeviceChannels;
    numOutputChannels = audioConfig.outputDeviceChannels;
    
    const unsigned int inputSampleRate = getBestInputSampleRate(inputSampleRates, audioConfig.sampleRate);
    if(inputSampleRate == 0)
    {
        throw ohmcomm::configuration_error("Resampling", std::string("Could not determine matching sample-rate to resample to: ") + std::to_string(audioConfig.sampleRate));
    }
    std::cout << "Resampling: Converting sample-rate of " << inputSampleRate << " to " << audioConfig.sampleRate << std::endl;
    samplesFactor = inputSampleRate / audioConfig.sampleRate;
    
    switch(audioConfig.audioFormatFlag)
    {
        case AudioConfiguration::AUDIO_FORMAT_SINT8:
            audioFormatSize = sizeof(int8_t);
            compressFunc = &Resampler::compress<int8_t>;
            extrapolateFunc = &Resampler::extrapolate<int8_t>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_SINT16:
            audioFormatSize = sizeof(int16_t);
            compressFunc = &Resampler::compress<int16_t>;
            extrapolateFunc = &Resampler::extrapolate<int16_t>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_SINT32:
            audioFormatSize = sizeof(int32_t);
            compressFunc = &Resampler::compress<int32_t>;
            extrapolateFunc = &Resampler::extrapolate<int32_t>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_FLOAT32:
            audioFormatSize = sizeof(float);
            compressFunc = &Resampler::compress<float>;
            extrapolateFunc = &Resampler::extrapolate<float>;
            break;
        case AudioConfiguration::AUDIO_FORMAT_FLOAT64:
            audioFormatSize = sizeof(double);
            compressFunc = &Resampler::compress<double>;
            extrapolateFunc = &Resampler::extrapolate<double>;
            break;
    }
}

unsigned int Resampler::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    const uint16_t numFrames = compressFunc(inputBuffer, inputBuffer, userData->nBufferFrames, numInputChannels, samplesFactor);
    userData->nBufferFrames = numFrames;
    return audioFormatSize * numInputChannels * numFrames;
}

unsigned int Resampler::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    if(writeBuffer == nullptr)
    {
        writeBuffer = new char[userData->maxBufferSize];
    }
    //since we increase the number of valid bytes in the buffer, we can't edit them in-place
    memcpy(writeBuffer, outputBuffer, outputBufferByteSize);
    const uint16_t numFrames = extrapolateFunc(writeBuffer, outputBuffer, userData->nBufferFrames, numOutputChannels, samplesFactor);
    userData->nBufferFrames = numFrames;
    return audioFormatSize * numOutputChannels * numFrames;
}

bool Resampler::cleanUp()
{
    if(writeBuffer != nullptr)
        delete[] writeBuffer;
    writeBuffer = nullptr;
    
    return true;
}

unsigned int Resampler::getBestInputSampleRate(const std::vector<unsigned int>& availableSampleRates, const unsigned int outputSampleRate)
{
    unsigned int bestInputSampleRate = AudioConfiguration::SAMPLE_RATE_ALL;
    //select the lowest available sample-rate multiple of the chosen from audioConfig
    for(const unsigned int availableSampleRate : availableSampleRates)
    {
        if(availableSampleRate % outputSampleRate == 0)
        {
            if(availableSampleRate < bestInputSampleRate)
            {
                bestInputSampleRate = availableSampleRate;
            }
        }
    }
    if(bestInputSampleRate == AudioConfiguration::SAMPLE_RATE_ALL)
        return 0;
    return bestInputSampleRate;
}

template<typename AudioFormat>
unsigned int Resampler::compress(const void* input, void* output, unsigned int numSamples, uint8_t numChannels, uint8_t factor)
{
    //left channel: abcde...
    //right channel: 1234...
    //input: a1a1a1a1b2b2b2b2c3c3c3c3...
    //output: a1b2c3...
    const AudioFormat* inputBuffer = (const AudioFormat*)input;
    AudioFormat* outputBuffer = (AudioFormat*) output;
    
    //iterate over all frames
    for(unsigned int i = 0; i < numSamples / factor; ++i)
    {
        //iterate over all channels in a single frame
        for(uint8_t c = 0; c < numChannels; ++c)
        {
            //calculates the arithmetic mean of #factor samples and stores it back inside the buffer
            //mean = 1 / n * sum(n samples)
            // = 1 / (n-1) * sum((n-1) samples) + 1 / n * sample n
            // = ... = sum((n-1) samples) * (n-1) / n + sample n / n
            AudioFormat tmp = 0;
            for(uint8_t s = 0; s < factor; ++s)
            {
                //use online method to prevent overflow
                //XXX but this method uses far more multiplications and divisions which could lead to rounding errors
                tmp = ((s>0) ? tmp * (s-1) / s : 0) + inputBuffer[i * factor * numChannels + c + s * numChannels] / (s + 1);
            }
            //store compressed value in output for frame/channel
            outputBuffer[i * numChannels + c] = tmp;
        }
    }
    //return new number of frames
    return numSamples / factor;
}

template<typename AudioFormat>
unsigned int Resampler::extrapolate(const void* input, void* output, unsigned int numSamples, uint8_t numChannels, uint8_t factor)
{
    //left channel: abcde...
    //right channel: 1234...
    //input: a1b2c3...
    //output: a1a1a1a1b2b2b2b2c3c3c3c3...
    const AudioFormat* inputBuffer = (const AudioFormat*)input;
    AudioFormat* outputBuffer = (AudioFormat*) output;
    
    //iterate over all frames
    for(unsigned int i = 0; i < numSamples; ++i)
    {
        //iterate over all channels in a frame
        for(uint8_t c = 0; c < numChannels; ++c)
        {
            //set #factor samples in the output to the same value
            for(uint8_t s = 0; s < factor; ++s)
            {
                outputBuffer[i * factor * numChannels + c + s * numChannels] = inputBuffer[i * numChannels + c];
            }
        }
    }
    //return new number of frames
    return numSamples * factor;
}

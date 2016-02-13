/*
 * File:   NetworkWrappingProcessor.cpp
 * Author: daniel
 *
 * Created on June 14, 2015, 10:05 AM
 */

#include "network/NetworkWrappingProcessor.h"

NetworkWrappingProcessor::NetworkWrappingProcessor(std::string name, NetworkWrapper *wrapper): AudioProcessor(name), wrapper(wrapper)
{
}

NetworkWrappingProcessor::~NetworkWrappingProcessor()
{
    delete wrapper;
}

unsigned int NetworkWrappingProcessor::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_ALL;
}

unsigned int NetworkWrappingProcessor::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

unsigned int NetworkWrappingProcessor::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    wrapper->sendData(inputBuffer, inputBufferByteSize);

    //no changes in buffer-size
    return inputBufferByteSize;
}

unsigned int NetworkWrappingProcessor::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    int receiveBufferSize = wrapper->receiveData(outputBuffer, outputBufferByteSize);
    while(receiveBufferSize== NetworkWrapper::RECEIVE_TIMEOUT)
    {
        receiveBufferSize = wrapper->receiveData(outputBuffer, outputBufferByteSize);
    }

    //set initial received buffer size
    return (unsigned int)receiveBufferSize;
}


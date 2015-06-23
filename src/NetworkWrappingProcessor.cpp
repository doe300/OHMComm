/* 
 * File:   NetworkWrappingProcessor.cpp
 * Author: daniel
 * 
 * Created on June 14, 2015, 10:05 AM
 */

#include "NetworkWrappingProcessor.h"

NetworkWrappingProcessor::NetworkWrappingProcessor(std::string name, NetworkWrapper *wrapper): AudioProcessor(name), wrapper(wrapper)
{
}

NetworkWrappingProcessor::~NetworkWrappingProcessor()
{
    delete wrapper;
}

unsigned int NetworkWrappingProcessor::getSupportedAudioFormats()
{
    return AudioConfiguration::AUDIO_FORMAT_ALL;
}

unsigned int NetworkWrappingProcessor::getSupportedSampleRates()
{
    return AudioConfiguration::SAMPLE_RATE_ALL;
}

unsigned int NetworkWrappingProcessor::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    wrapper->sendDataNetworkWrapper(inputBuffer, inputBufferByteSize); 
    
    //no changes in buffer-size
    return inputBufferByteSize;
}

unsigned int NetworkWrappingProcessor::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    wrapper->recvDataNetworkWrapper(outputBuffer, outputBufferByteSize);
    
    //no changes in buffer-size
    return outputBufferByteSize;
}


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

void NetworkWrappingProcessor::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    wrapper->sendDataNetworkWrapper(inputBuffer, inputBufferByteSize); 
}

void NetworkWrappingProcessor::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    wrapper->recvDataNetworkWrapper(outputBuffer, outputBufferByteSize);
}


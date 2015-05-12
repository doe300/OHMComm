/* 
 * File:   RTPWrapper.cpp
 * Author: daniel
 * 
 * Created on March 7, 2015, 1:10 PM
 */

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "RTPWrapper.h"

RTPWrapper::RTPWrapper() : NetworkWrapper()
{
    //initialize random start values
    synchronizationSource = generateSynchronizationSource();
    currentSequenceNumber = generateSequenceNumber();
    startTimestamp = generateTimestamp();
    contributionSourcesCount = 0;
    payloadType = L16_2;
}

RTPWrapper::RTPWrapper(const RTPWrapper& orig) : NetworkWrapper(orig)
{
    RTPWrapper::socket = orig.socket;
    synchronizationSource = orig.synchronizationSource;
    currentSequenceNumber = orig.currentSequenceNumber;
    startTimestamp = orig.startTimestamp;
    //copies the other wrapper's contribution sources
    memcpy(contributionSources, orig.contributionSources, sizeof(contributionSources));
    contributionSourcesCount = orig.contributionSourcesCount;
    payloadType = orig.payloadType;
}

RTPWrapper::~RTPWrapper()
{
    //socket is closed by ~NetworkWrapper()
}

void RTPWrapper::configure()
{
    //initialize socket
    if(initializeNetwork() != 0)
    {
        std::cerr << "Failed to initialize the socket!" << std::endl;
        return;
    }
    inputFrameSize = getBytesFromAudioFormat(audioConfiguration.InputAudioFormat);
    outputFrameSize = getBytesFromAudioFormat(audioConfiguration.OutputAudioFormat);
    
    //TODO configure payload-type
    
    //the maximum size of the buffer is: nFrames * frameSize + maximum size of header (currently only without extension)
    sendBuffer = (char *)malloc(getBufferSize(audioConfiguration.bufferFrames, inputFrameSize, audioConfiguration.InputDeviceChannels) + RTP_HEADER_MAX_SIZE);
    receiveBuffer = (char *)malloc(getBufferSize(audioConfiguration.bufferFrames, outputFrameSize, audioConfiguration.OutputDeviceChannels) + RTP_HEADER_MAX_SIZE);
    if(sendBuffer == NULL || receiveBuffer == NULL)
    {
        throw(std::bad_alloc());
    }
}


////
// Send/Receive
////

int RTPWrapper::process(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status, void* data)
{
    if(inputBuffer != NULL)
    {
        /*
         * 1. pack input buffer into package
         */
        //1.1 build header
        RTPHeader header;
        header.padding = 0;
        header.extension = 0;
        header.csrc_count = getContributionSourcesCount();
        header.marker = 0;
        header.payload_type = payloadType;
        header.sequence_number = getSequenceNumber();
        header.timestamp = getTimestamp(streamTime);
        header.ssrc = getSynchronizationSource();
        memcpy(header.csrc_list, contributionSources, sizeof(contributionSources));
        //1.2 set body
        size_t packageSize = getBufferSize(nBufferFrames, inputFrameSize, audioConfiguration.InputDeviceChannels);
        RTPPackage package(header, packageSize, (uint8_t *)inputBuffer);
        //1.3 write package
        packageSize = package.copyToBuffer(sendBuffer, sizeof(sendBuffer));
        std::cout << "Sent sequence number: " << header.sequence_number << std::endl;
        long int size = sendto(Socket, sendBuffer, packageSize, 0, &networkConfiguration.remoteAddr, sizeof(networkConfiguration.remoteAddr));
        if(size == -1)
        {
            std::cerr << "Error while sending RTP Package: " << errno << std::endl;
            //cancel immediately
            return 1;
        }
        std::cout << "Sent: " << size << std::endl;
        
        incrementSequenceNumber();
    }
    
    if(outputBuffer != NULL)
    {
        /*
         * 2. read input into receiveBuffer
         */
        //need to receive whole size of buffer, not just the payload!! (-> add header)
        long unsigned int outputBufferSize = getBufferSize(nBufferFrames, outputFrameSize, audioConfiguration.OutputDeviceChannels) + RTP_HEADER_MAX_SIZE;
        long int size = recvfrom(Socket, receiveBuffer, outputBufferSize, 0, NULL, 0);
        if(size == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "No data available" << std::endl;
            }
            else
            {
                std::cerr << "Error while receiving RTP package: " << errno << std::endl;
                //cancel immediately
                return 1;
            }
        }
        std::cout << "Received: " << size << std::endl;
        //2.1 extract package
        RTPPackage package;
        //TODO or reuse buffer (allocate to maximum and only use up to size) ??
        package.readFromBuffer(receiveBuffer, size);
        std::cout << "Received sequence number: " << package.header.sequence_number << std::endl;
        //2.3. write body to outputBuffer
        memcpy(outputBuffer, package.package, package.packageSize);
        //free package buffer!
        free(package.package);
    }
    return 0;
}

////
// Sender configuration
////

int8_t RTPWrapper::addContributionSource(uint32_t csrc)
{
    if(getContributionSourcesCount() < 15)
    {
        for(uint8_t i = 0;i < getContributionSourcesCount(); i++)
        {
            if(contributionSources[i] == 0)
            {
                contributionSources[i] = csrc;
                contributionSourcesCount++;
                return 0;
            }
        }
    }
    return -1;
}

uint8_t RTPWrapper::getContributionSourcesCount()
{
    return contributionSourcesCount;
}

uint16_t RTPWrapper::getSequenceNumber()
{
    return currentSequenceNumber;
}

uint32_t RTPWrapper::getSynchronizationSource()
{
    return synchronizationSource;
}


////
// private helper methods
////

uint16_t RTPWrapper::generateSequenceNumber()
{
    //TODO better random??
    return rand() & 0xFFFF;
}

uint32_t RTPWrapper::generateSynchronizationSource()
{
    //TODO better random??
    return rand() & 0xFFFFFFFF;
}

uint32_t RTPWrapper::generateTimestamp()
{
    //TODO better random??
    return rand() & 0xFFFFFFFF;
}

uint16_t RTPWrapper::incrementSequenceNumber()
{
    //TODO overflow handling - required?
    return currentSequenceNumber++;
}

uint32_t RTPWrapper::getTimestamp(double streamTime)
{
    //timestamp is in milliseconds
    return startTimestamp + (int) (streamTime * 1000);
}

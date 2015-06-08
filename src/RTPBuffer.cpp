/* 
 * File:   RTPBuffer.cpp
 * Author: daniel
 * 
 * Created on March 28, 2015, 12:27 PM
 */

#include "RTPBuffer.h"
#include "configuration.h"

#include <iostream>

//TODO implement maxDelay (problem: RTP-timestamp is not guaranteed to be in ms or ns or s, ...)

RTPBuffer::RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay): capacity(maxCapacity), maxDelay(maxDelay)
{
    nextReadIndex = 0;
    ringBuffer = new RTPBufferPackage[maxCapacity];
    size = 0;
    minSequenceNumber = 0;
    //initialize silence package
    generateSilencePackage();
    #ifdef _WIN32
    bufferMutex = CreateMutex(NULL, false, "BufferMutex");
    #endif
}

RTPBuffer::RTPBuffer(const RTPBuffer& orig): capacity(orig.capacity), maxDelay(orig.maxDelay)
{
    nextReadIndex = orig.nextReadIndex;
    ringBuffer = orig.ringBuffer;
    size = orig.size;
    minSequenceNumber = orig.minSequenceNumber;
    silencePackage = orig.silencePackage;
}

RTPBuffer::~RTPBuffer()
{
    delete [] ringBuffer;
}

RTPBufferStatus RTPBuffer::addPackage(RTPPackage &package, unsigned int contentSize)
{
    lockMutex();
    RTPHeader *receivedHeader = (RTPHeader *)package.getHeaderFromRTPPackage();
    if(minSequenceNumber == 0)
    {
        //if we receive our first package, we need to set minSequenceNumber
        minSequenceNumber = receivedHeader->sequence_number;
    }
    if(receivedHeader->sequence_number < minSequenceNumber)
    {
        //discard package, because it is older than the minimum sequence number to hold
        unlockMutex();
        return RTP_BUFFER_ALL_OKAY;
    }
    if(size == capacity)
    {
        //buffer is full
        unlockMutex();
        return RTP_BUFFER_INPUT_OVERFLOW;
    }
    if(receivedHeader->sequence_number - minSequenceNumber >= capacity)
    {
        //package is far too new -> we have now choice but to discard it without getting into an undetermined state
        unlockMutex();
        return RTP_BUFFER_INPUT_OVERFLOW;
    }
    uint16_t newWriteIndex = calculateIndex(nextReadIndex, receivedHeader->sequence_number-minSequenceNumber);
    //write package-data into buffer
    ringBuffer[newWriteIndex].isValid = true;
    ringBuffer[newWriteIndex].header = *receivedHeader;
    ringBuffer[newWriteIndex].contentSize = contentSize;
    if(ringBuffer[newWriteIndex].packageContent == NULL)
    {
        //if first time writing to this buffer-cell, allocate space
        ringBuffer[newWriteIndex].packageContent = new char[contentSize];
    }
    memcpy(ringBuffer[newWriteIndex].packageContent, package.getDataFromRTPPackage(), contentSize);
    //update size
    size++;
    unlockMutex();
    return RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBuffer::readPackage(RTPPackage &package)
{
    lockMutex();
    if(size <= 0)
    {
        //buffer is empty
        //write placeholder package
        char *packageBuffer = (char *)package.getRecvBuffer();
        memcpy(packageBuffer, &(silencePackage.header), sizeof(silencePackage.header));
        memcpy(packageBuffer + sizeof(silencePackage.header), silencePackage.packageContent, silencePackage.contentSize);
        unlockMutex();
        return RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    //need to search for oldest valid package, newer than minSequenceNumber
    uint16_t index = nextReadIndex;
    while(incrementIndex(index) != nextReadIndex)
    {
        if(ringBuffer[index].isValid == true && ringBuffer[index].header.sequence_number >= minSequenceNumber)
        {
            nextReadIndex = index;
            break;
        }
        index = incrementIndex(index);
    }
    //This copies the content of ringBuffer[readIndex] into package
    RTPBufferPackage *bufferPack = &(ringBuffer[nextReadIndex]);
    if(bufferPack->isValid == false)
    {
        //no valid packages found -> should never occur, because size was > 0
        unlockMutex();
        return RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    char *packageBuffer = (char *)package.getRecvBuffer();
    memcpy(packageBuffer, &(bufferPack->header), sizeof(bufferPack->header));
    memcpy(packageBuffer + sizeof(bufferPack->header), bufferPack->packageContent, bufferPack->contentSize);
    //Invalidate buffer-entry
    bufferPack->isValid = false;
    //Increment Index, decrease size
    nextReadIndex = incrementIndex(nextReadIndex);
    size--;
    //only accept newer packages (at least one sequence number more than last read package)
    minSequenceNumber = (bufferPack->header.sequence_number + 1) % UINT16_MAX;
    unlockMutex();
    return RTP_BUFFER_ALL_OKAY;
}

uint16_t RTPBuffer::getSize()
{
    return size;
}

uint16_t RTPBuffer::calculateIndex(uint16_t index, uint16_t offset)
{
    return (index + offset) % capacity;
}

uint16_t RTPBuffer::incrementIndex(uint16_t index)
{
    return (index+1) % capacity;
}

void RTPBuffer::generateSilencePackage()
{
    RTPHeader dummyHeader;
    dummyHeader.version = 2;
    dummyHeader.padding = 0;
    dummyHeader.extension = 0;
    dummyHeader.csrc_count = 0;
    dummyHeader.marker = 0;
    //XXX how to get to those values?
    dummyHeader.payload_type = L16_2;
    dummyHeader.sequence_number = 0;
    dummyHeader.timestamp = 0;
    dummyHeader.ssrc = 0;
    
    //copy dummy header
    silencePackage.header = dummyHeader;
    silencePackage.isValid = true;
    silencePackage.contentSize = networkConfiguration.outputBufferSize;
    silencePackage.packageContent = new char[silencePackage.contentSize];
    //fill payload with zero
    memset(silencePackage.packageContent, 0, silencePackage.contentSize);
}

void RTPBuffer::lockMutex()
{
    #ifdef _WIN32
    WaitForSingleObject(bufferMutex, INFINITE);
    #else
    bufferMutex.lock();
    #endif
}

void RTPBuffer::unlockMutex()
{
    #ifdef _WIN32
    ReleaseMutex(bufferMutex);
    #else
    bufferMutex.unlock();
    #endif
}


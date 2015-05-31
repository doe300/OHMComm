/* 
 * File:   RTPBuffer.cpp
 * Author: daniel
 * 
 * Created on March 28, 2015, 12:27 PM
 */

#include "RTPBuffer.h"

#include <iostream>
#include <malloc.h>

//TODO implement maxDelay (problem: RTP-timestamp is not guaranteed to be in ms or ns or s, ...)

RTPBuffer::RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay): capacity(maxCapacity), maxDelay(maxDelay)
{
    nextReadIndex = 0;
    ringBuffer = (RTPBufferPackage *)malloc(sizeof(RTPBufferPackage) * maxCapacity);
    size = 0;
    minSequenceNumber = 0;
}

RTPBuffer::RTPBuffer(const RTPBuffer& orig): capacity(orig.capacity), maxDelay(orig.maxDelay)
{
    nextReadIndex = orig.nextReadIndex;
    ringBuffer = orig.ringBuffer;
    size = orig.size;
    minSequenceNumber = orig.minSequenceNumber;
}

RTPBuffer::~RTPBuffer()
{
}

RTPBufferStatus RTPBuffer::addPackage(RTPPackage &package, unsigned int contentSize)
{
    bufferMutex.lock();
    RTPHeader *receivedHeader = (RTPHeader *)package.getHeaderFromRTPPackage();
    if(receivedHeader->sequence_number < minSequenceNumber)
    {
        //discard package, because it is older than the minimum sequence number to hold
        return RTP_BUFFER_ALL_OKAY;
    }
    if(size == capacity)
    {
        //buffer is full
        return RTP_BUFFER_INPUT_OVERFLOW;
    }
    if(receivedHeader->sequence_number - minSequenceNumber >= capacity)
    {
        //package is far too new -> we have now choice but to discard it without getting into an undetermined state
        return RTP_BUFFER_INPUT_OVERFLOW;
    }
    uint16_t newWriteIndex = calculateIndex(nextReadIndex, receivedHeader->sequence_number-minSequenceNumber);
    //write package-data into buffer
    ringBuffer[newWriteIndex].isValid = true;
    ringBuffer[newWriteIndex].header = *receivedHeader;
    ringBuffer[newWriteIndex].contentSize = contentSize;
    if(ringBuffer[newWriteIndex].packageContent == NULL)
    {
        //if first time writing to this buffer-cell, malloc space
        ringBuffer[newWriteIndex].packageContent = malloc(contentSize);
    }
    memcpy(ringBuffer[newWriteIndex].packageContent, package.getDataFromRTPPackage(), contentSize);
    //update size
    size++;
    bufferMutex.unlock();
    return RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBuffer::readPackage(RTPPackage &package)
{
    bufferMutex.lock();
    if(size <= 0)
    {
        //buffer is empty
        return RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    //need to search for oldest valid package, newer than minSequenceNumber
    for(uint16_t index = nextReadIndex; index != nextReadIndex -1; index = index+1 % capacity)
    {
        if(ringBuffer[index].isValid == true && ringBuffer[index].header.sequence_number >= minSequenceNumber)
        {
            nextReadIndex = index;
            break;
        }
    }
    //This copies the content of ringBuffer[readIndex] into package
    RTPBufferPackage *bufferPack = &(ringBuffer[nextReadIndex]);
    if(bufferPack->isValid == false)
    {
        //no valid packages found -> should never occur, because size was > 0
        return RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    void *packageBuffer = package.getRecvBuffer();
    memcpy(packageBuffer, &(bufferPack->header), sizeof(bufferPack->header));
    memcpy(packageBuffer, bufferPack->packageContent, bufferPack->contentSize);
    //Invalidate buffer-entry
    bufferPack->isValid = false;
    //Increment Index, decrease size
    nextReadIndex = incrementIndex(nextReadIndex);
    size--;
    //only accept newer packages (at least one sequence number more than last read package)
    minSequenceNumber = (bufferPack->header.sequence_number + 1) % capacity;
    bufferMutex.unlock();
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
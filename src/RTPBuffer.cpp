/* 
 * File:   RTPBuffer.cpp
 * Author: daniel
 * 
 * Created on March 28, 2015, 12:27 PM
 */

#include "RTPBuffer.h"
#include "configuration.h"

#include <iostream>

//TODO implement maxDelay

RTPBuffer::RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages): capacity(maxCapacity), maxDelay(maxDelay), minBufferPackages(minBufferPackages)
{
    nextReadIndex = 0;
    ringBuffer = new RTPBufferPackage[maxCapacity];
    size = 0;
    minSequenceNumber = 0;
    #ifdef _WIN32
    bufferMutex = CreateMutex(nullptr, false, "BufferMutex");
    #endif
}

RTPBuffer::~RTPBuffer()
{
    delete [] ringBuffer;
}

RTPBufferStatus RTPBuffer::addPackage(RTPPackageHandler &package, unsigned int contentSize)
{
    lockMutex();
    RTPHeader *receivedHeader = package.getRTPPackageHeader();
    if(minSequenceNumber == 0)
    {
        //if we receive our first package, we need to set minSequenceNumber
        minSequenceNumber = receivedHeader->sequence_number;
    }

    //we need to check for upper limit of range, because at some point a wrap around UINT16_MAX is expected behavior
    // -> if minSequenceNumber is larger than (UINT16_MAX - capacity), sequence_number around zero have to be allowed for
    if(minSequenceNumber < (UINT16_MAX - capacity) && receivedHeader->sequence_number < minSequenceNumber)
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
        // TODO: Its not smart to discard new packages. This is only the case if packages were lost while transmitting. 
		// And if that is the case then the buffer should not wait for old packages and discard all further (new) packages. 
		// Instead it should the move the current pointer in the buffer ahead. 
        // -> need to correctly implement maxDelay

        //package is far too new -> we have now choice but to discard it without getting into an undetermined state
        unlockMutex();
        return RTP_BUFFER_INPUT_OVERFLOW;
    }
    uint16_t newWriteIndex = calculateIndex(nextReadIndex, receivedHeader->sequence_number-minSequenceNumber);
    //write package-data into buffer
    ringBuffer[newWriteIndex].isValid = true;

    ringBuffer[newWriteIndex].header = *receivedHeader;

    ringBuffer[newWriteIndex].contentSize = contentSize;

    if(ringBuffer[newWriteIndex].packageContent == nullptr)
    {
        ringBuffer[newWriteIndex].packageContent = new char[contentSize];
    }
    memcpy(ringBuffer[newWriteIndex].packageContent, package.getRTPPackageData(), contentSize);
    //update size
    size++;
    unlockMutex();
    return RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBuffer::readPackage(RTPPackageHandler &package)
{
    lockMutex();
    if(size < minBufferPackages)
    {
        //buffer is empty
        //write placeholder package into buffer
        package.createSilencePackage();
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

    // TODO: move to RTPPackage? need to use correct buffer!
    char *packageBuffer = (char *)package.getWorkBuffer();
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


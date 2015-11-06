/*
 * File:   RTPBuffer.cpp
 * Author: daniel
 *
 * Created on March 28, 2015, 12:27 PM
 */


#include "RTPBuffer.h"


RTPBuffer::RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages) : capacity(maxCapacity), maxDelay(maxDelay), minBufferPackages(minBufferPackages)
{
    nextReadIndex = 0;
    ringBuffer = new RTPBufferPackage[maxCapacity];
    size = 0;
    minSequenceNumber = 0;
    #ifdef _WIN32
    bufferMutex = CreateMutex(nullptr, false, L"BufferMutex");
    #endif
    Statistics::setCounter(Statistics::RTP_BUFFER_LIMIT, maxCapacity);
}

RTPBuffer::~RTPBuffer()
{
    delete [] ringBuffer;
}

RTPBufferStatus RTPBuffer::addPackage(RTPPackageHandler &package, unsigned int contentSize)
{
    lockMutex();
    const RTPHeader *receivedHeader = package.getRTPPackageHeader();
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
        return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
    }

    if(size == capacity)
    {
        //buffer is full
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;
    }
    if(receivedHeader->sequence_number - minSequenceNumber >= capacity)
    {
        //should never occur: package is far too new -> we have now choice but to discard it without getting into an undetermined state
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;
    }
    uint16_t newWriteIndex = calculateIndex(nextReadIndex, receivedHeader->sequence_number-minSequenceNumber);
    //write package-data into buffer
    ringBuffer[newWriteIndex].isValid = true;
    ringBuffer[newWriteIndex].header = *receivedHeader;
    if(ringBuffer[newWriteIndex].packageContent == nullptr)
    {
        //allocate new buffer with the current content-size
        ringBuffer[newWriteIndex].bufferSize = contentSize;
        ringBuffer[newWriteIndex].packageContent = malloc(contentSize);
    }
    else if(ringBuffer[newWriteIndex].bufferSize < contentSize)
    {
        //reallocate buffer, because the content would not fit
        ringBuffer[newWriteIndex].bufferSize = contentSize;
        ringBuffer[newWriteIndex].packageContent = realloc(ringBuffer[newWriteIndex].packageContent, contentSize);
    }
    //save timestamp of reception
    ringBuffer[newWriteIndex].receptionTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    ringBuffer[newWriteIndex].contentSize = contentSize;
    memcpy(ringBuffer[newWriteIndex].packageContent, package.getRTPPackageData(), contentSize);
    //update size
    size++;
    Statistics::maxCounter(Statistics::RTP_BUFFER_MAXIMUM_USAGE, size);
    unlockMutex();
    return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBuffer::readPackage(RTPPackageHandler &package)
{
    lockMutex();
    if(size < minBufferPackages)
    {
        //buffer has insufficient fill level
        //return silence package
        package.createSilencePackage();
        package.setActualPayloadSize(package.getMaximumPackageSize());
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    //need to search for oldest valid package, newer than minSequenceNumber and newer than currentTimestamp - maxDelay
    uint16_t index = nextReadIndex;
    unsigned long currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    while(incrementIndex(index) != nextReadIndex)
    {
        //check whether package is too delayed
        if(ringBuffer[index].isValid == true && ringBuffer[index].receptionTimestamp + maxDelay < currentTimestamp)
        {
            //package is valid but too old, invalidate and skip
            ringBuffer[index].isValid = false;
        }
        else if(ringBuffer[index].isValid == true && ringBuffer[index].header.sequence_number >= minSequenceNumber)
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
        //no valid packages found
        //return silence package
        package.createSilencePackage();
        package.setActualPayloadSize(package.getMaximumPackageSize());
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW;
    }

    char *packageBuffer = (char *)package.getWorkBuffer();
    memcpy(packageBuffer, &(bufferPack->header), sizeof(bufferPack->header));
    memcpy(packageBuffer + sizeof(bufferPack->header), bufferPack->packageContent, bufferPack->contentSize);
    package.setActualPayloadSize(bufferPack->contentSize);

    //Invalidate buffer-entry
    bufferPack->isValid = false;
    //Increment Index, decrease size
    nextReadIndex = incrementIndex(nextReadIndex);
    size--;
    //we lost all packages between the last read and this one, so we subtract the sequence numbers
    Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_LOST, (bufferPack->header.sequence_number - minSequenceNumber)%UINT16_MAX);
    //only accept newer packages (at least one sequence number more than last read package)
    minSequenceNumber = (bufferPack->header.sequence_number + 1) % UINT16_MAX;
    unlockMutex();
    return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

unsigned int RTPBuffer::getSize() const
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


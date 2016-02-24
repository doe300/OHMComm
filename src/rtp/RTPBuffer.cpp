/*
 * File:   RTPBuffer.cpp
 * Author: daniel
 *
 * Created on March 28, 2015, 12:27 PM
 */


#include "rtp/RTPBuffer.h"


RTPBuffer::RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages) : PlayoutPointAdaption(200, minBufferPackages),
    capacity(maxCapacity), maxDelay(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(maxDelay)))
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

RTPBufferStatus RTPBuffer::addPackage(const RTPPackageHandler &package, unsigned int contentSize)
{
    lockMutex();
    const RTPHeader *receivedHeader = package.getRTPPackageHeader();
    if(minSequenceNumber == 0 || receivedHeader->isMarked())
    {
        //if we receive our first package, we need to set minSequenceNumber
        //same for the first package after a silent period
        minSequenceNumber = receivedHeader->getSequenceNumber();
        //TODO also, if we lost more then a certain number of consecutive packages, accept the next one
        //(if its sequence-number is more than the last not-lost)
        //-> this allows for continuation of communication for DTX, even if marked package gets lost
    }

    //we need to check for upper limit of range, because at some point a wrap around UINT16_MAX is expected behavior
    // -> if minSequenceNumber is larger than (UINT16_MAX - capacity), sequence_number around zero have to be allowed for
    if(minSequenceNumber < (UINT16_MAX - capacity) && receivedHeader->getSequenceNumber() < minSequenceNumber)
    {
        //late loss
        //discard package, because it is older than the minimum sequence number to hold
        packageReceived(true);
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
    }

    if(size == capacity)
    {
        //buffer is full
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;
    }
    if(receivedHeader->getSequenceNumber() - minSequenceNumber >= capacity)
    {
        //should never occur: package is far too new -> we have now choice but to discard it without getting into an undetermined state
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;
    }
    uint16_t newWriteIndex = calculateIndex(nextReadIndex, receivedHeader->getSequenceNumber()-minSequenceNumber);
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
    ringBuffer[newWriteIndex].receptionTimestamp = std::chrono::steady_clock::now();
    ringBuffer[newWriteIndex].contentSize = contentSize;
    memcpy(ringBuffer[newWriteIndex].packageContent, package.getRTPPackageData(), contentSize);
    //update size
    size++;
    Statistics::maxCounter(Statistics::RTP_BUFFER_MAXIMUM_USAGE, size);
    packageReceived(false);
    unlockMutex();
    return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBuffer::readPackage(RTPPackageHandler &package)
{
    lockMutex();
    if(!isAdaptionBufferFilled())
    {
        //buffer has insufficient fill level
        //return concealment package
        createConcealmentPackage(package);
        //we do not increase the minimum sequence number here, because we want to stretch the play-out delay
        //for that, we need to insert, not replace packages
        unlockMutex();
        return RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    //need to search for oldest valid package, newer than minSequenceNumber and newer than currentTimestamp - maxDelay
    uint16_t index = nextReadIndex;
    const std::chrono::steady_clock::time_point currentTimestamp = std::chrono::steady_clock::now();
    while(incrementIndex(index) != nextReadIndex)
    {
        //check whether package is too delayed
        if(ringBuffer[index].isValid == true && ringBuffer[index].receptionTimestamp + maxDelay < currentTimestamp)
        {
            //package is valid but too old, invalidate and skip
            ringBuffer[index].isValid = false;
        }
        else if(ringBuffer[index].isValid == true && ringBuffer[index].header.getSequenceNumber() >= minSequenceNumber)
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
        //no valid packages found -> buffer is empty
        //return concealment package
        concealLoss(package, minSequenceNumber);
        //only accept newer packages (at least one sequence number more than the dummy package)
        //but skip check for first package
        if(minSequenceNumber != 0)
            minSequenceNumber = (minSequenceNumber + 1) % UINT16_MAX;
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
    Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_LOST, (bufferPack->header.getSequenceNumber() - minSequenceNumber)%UINT16_MAX);
    //only accept newer packages (at least one sequence number more than last read package)
    minSequenceNumber = (bufferPack->header.getSequenceNumber() + 1) % UINT16_MAX;
    unlockMutex();
    return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

unsigned int RTPBuffer::getSize() const
{
    return size;
}

bool RTPBuffer::repeatLastPackage(RTPPackageHandler& package, const uint16_t packageSequenceNumber)
{
    //reverse iterate the buffer to get to the position for the given sequence-number
    uint16_t index = nextReadIndex;
    while(index != incrementIndex(nextReadIndex))
    {
        if(ringBuffer[index].header.getSequenceNumber() == packageSequenceNumber)
        {
            RTPBufferPackage *bufferPack = &(ringBuffer[index]);
            char *packageBuffer = (char *)package.getWorkBuffer();
            memcpy(packageBuffer, &(bufferPack->header), sizeof(bufferPack->header));
            memcpy(packageBuffer + sizeof(bufferPack->header), bufferPack->packageContent, bufferPack->contentSize);
            package.setActualPayloadSize(bufferPack->contentSize);
            return true;
        }
        index = index == 0 ? capacity : index-1;
    }
    return false;
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


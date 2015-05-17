///* 
// * File:   RTPBuffer.cpp
// * Author: daniel
// * 
// * Created on March 28, 2015, 12:27 PM
// */
//
//#include "RTPBuffer.h"
//
////TODO implement maxDelay (and ordering by sequence number)
//
//RTPBuffer::RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay)
//{
//    RTPBuffer::capacity = maxCapacity;
//    RTPBuffer::maxDelay = maxCapacity;
//    RTPBuffer::readIndex = 0;  //read is enabled at first write
//    RTPBuffer::writeIndex = 0;
//    RTPBuffer::ringBuffer = new RTPPackage[maxCapacity];
//    RTPBuffer::size = 0;
//}
//
//RTPBuffer::RTPBuffer(const RTPBuffer& orig)
//{
//    capacity = orig.capacity;
//    maxDelay = orig.maxDelay;
//    readIndex = orig.readIndex;
//    writeIndex = orig.writeIndex;
//}
//
//RTPBuffer::~RTPBuffer()
//{
//}
//
//uint16_t RTPBuffer::incrementIndex(uint16_t index)
//{
//    if(index == capacity-1)
//    {
//        //overflow, start at beginning
//        return 0;
//    }
//    return index+1;
//}
//
//
//RTPBufferStatus RTPBuffer::addPackage(const RTPPackage &package)
//{
//    if(size != 0 && writeIndex == readIndex)
//    {
//        //would overwrite used buffer space
//        //if size is 0, both indices are the same, but no data to override
//        return RTP_BUFFER_INPUT_OVERFLOW;
//    }
//    ringBuffer[writeIndex] = package;
//    writeIndex = incrementIndex(writeIndex);
//    size++;
//    return RTP_BUFFER_ALL_OKAY;
//}
//
//RTPBufferStatus RTPBuffer::readPackage(RTPPackage &package)
//{
//    if(readIndex == writeIndex)
//    {
//        //buffer is empty
//        return RTP_BUFFER_OUTPUT_UNDERFLOW;
//    }
//    //This copies the content of ringBuffer[readIndex] into package
//    //more precisely: copies the two pointers inRTPPackage into the memory allocated for package
//    package = ringBuffer[readIndex];
//    readIndex = incrementIndex(readIndex);
//    size--;
//    return RTP_BUFFER_ALL_OKAY;
//}
//
//uint16_t RTPBuffer::getSize()
//{
//    return size;
//}
//

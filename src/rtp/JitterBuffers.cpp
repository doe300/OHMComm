/* 
 * File:   JitterBuffers.cpp
 * Author: daniel
 * 
 * Created on February 25, 2016, 2:00 PM
 */

#include "rtp/JitterBuffers.h"
#include "rtp/RTPBuffer.h"

JitterBuffers::JitterBuffers(const uint16_t maxCapacity, const uint16_t maxDelay, const uint16_t minBufferPackage) :
    maximumCapacity(maxCapacity), maximumDelay(maxDelay), minBufferPackages(minBufferPackage)
{

}

const std::unique_ptr<RTPBufferHandler>& JitterBuffers::getBuffer(const uint32_t ssrc)
{
    //needs to by locked due to direct access by 2 threads
    mutex.lock();
    if(buffers.find(ssrc) == buffers.end())
    {
        buffers.insert(std::pair<uint32_t, std::unique_ptr<RTPBufferHandler>>(ssrc, std::unique_ptr<RTPBufferHandler>(new RTPBuffer(ssrc, maximumCapacity, maximumDelay, minBufferPackages))));
    }
    mutex.unlock();
    return buffers.at(ssrc);
}

void JitterBuffers::removeBuffer(const uint32_t ssrc)
{
    mutex.lock();
    buffers.erase(ssrc);
    mutex.unlock();
}

void JitterBuffers::cleanup()
{
    mutex.lock();
    buffers.clear();
    mutex.unlock();
}

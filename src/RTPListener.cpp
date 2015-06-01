/* 
 * File:   ReceiveThread.cpp
 * Author: daniel
 * 
 * Created on May 16, 2015, 12:49 PM
 */

#include "RTPListener.h"

RTPListener::RTPListener(NetworkWrapper *wrapper, RTPBuffer *buffer, unsigned int receiveBufferSize)
{
    this->wrapper = wrapper;
    this->buffer = buffer;
    receivedPackage = new RTPPackage(receiveBufferSize);
}

RTPListener::RTPListener(const RTPListener& orig)
{
    this->wrapper = orig.wrapper;
    this->buffer = orig.buffer;
}

RTPListener::~RTPListener()
{
    shutdown();
}

void RTPListener::startUp()
{
    threadRunning = true;
    receiveThread = std::thread(&RTPListener::runThread, this);
}

void RTPListener::runThread()
{
    while(threadRunning)
    {
        //1. wait for package and store into RTPPackage
        int receivedSize = this->wrapper->recvDataNetworkWrapper(receivedPackage->getRecvBuffer(), receivedPackage->getPacketSizeRTPPackage());
        if(receivedSize == EAGAIN || receivedSize == EWOULDBLOCK)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else
        {
            //2. write package to buffer
            if(buffer->addPackage(*receivedPackage, receivedSize - RTP_HEADER_MIN_SIZE) == RTP_BUFFER_INPUT_OVERFLOW)
            {
                //TODO some handling or simply discard?
                std::cerr << "Input Buffer overflow" << std::endl;
            }
        }
    }
}

void RTPListener::shutdown()
{
    //notify the thread to stop
    threadRunning = false;
}


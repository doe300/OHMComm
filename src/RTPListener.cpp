/* 
 * File:   ReceiveThread.cpp
 * Author: daniel
 * 
 * Created on May 16, 2015, 12:49 PM
 */

#include "RTPListener.h"

RTPListener::RTPListener(NetworkWrapper *wrapper, std::unique_ptr<RTPBuffer> *buffer, unsigned int receiveBufferSize)
{
    this->wrapper = wrapper;
    this->buffer = buffer;
	receivedPackage = new RTPPackageHandler(receiveBufferSize);
}

RTPListener::RTPListener(const RTPListener& orig)
{
    this->wrapper = orig.wrapper;
    this->buffer = orig.buffer;
}

RTPListener::~RTPListener()
{
    shutdown();
    delete receivedPackage;
}

void RTPListener::startUp()
{
    threadRunning = true;
    receiveThread = std::thread(&RTPListener::runThread, this);
}

void RTPListener::runThread()
{
    std::cout << "RTP-Listener started ..." << std::endl;
    while(threadRunning)
    {
        //1. wait for package and store into RTPPackage
        int receivedSize = this->wrapper->recvDataNetworkWrapper(receivedPackage->getWorkBuffer(), receivedPackage->getSize());
        if(receivedSize == EAGAIN || receivedSize == EWOULDBLOCK)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else
        {
            //2. write package to buffer
            if((*buffer)->addPackage(*receivedPackage, receivedSize - RTP_HEADER_MIN_SIZE) == RTP_BUFFER_INPUT_OVERFLOW)
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


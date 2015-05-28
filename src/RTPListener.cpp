/* 
 * File:   ReceiveThread.cpp
 * Author: daniel
 * 
 * Created on May 16, 2015, 12:49 PM
 */

#include "RTPListener.h"

RTPListener::RTPListener(const sockaddr *receiveAddress, RTPBuffer *buffer, unsigned int receiveBufferSize)
{
    this->receiveAddress = receiveAddress;
    this->buffer = buffer;
    receivedPackage = new RTPPackage(receiveBufferSize);
}

RTPListener::RTPListener(const RTPListener& orig)
{
    this->receiveAddress = orig.receiveAddress;
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
    int connectionType = networkConfiguration.connectionType == NetworkConfiguration::ConnectionType::TCP ? SOCK_STREAM : SOCK_DGRAM;
    receiveSocket = socket(AF_INET, connectionType, 0);
    if (bind(receiveSocket, receiveAddress, sizeof(receiveAddress)) == SOCKET_ERROR)
    {
            std::cerr << "Error binding the socket: " << NetworkWrapper::getLastError() << std::endl;
            return;
    }
    else
    {
            std::cout << "Local port bound." << std::endl;
    }
}

void RTPListener::runThread()
{
    while(receiveSocket >= 0 && threadRunning)
    {
        //1. wait for package and store into RTPPackage
        int receivedSize = recv(receiveSocket, (char *)receivedPackage->getRecvBuffer(), receivedPackage->getPacketSizeRTPPackage(), 0);
        if(receivedSize == EAGAIN || receivedSize == EWOULDBLOCK)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        //2. write package to buffer
        //buffer->addPackage(*receivedPackage, receivedSize - RTP_HEADER_MIN_SIZE);
    }
}

void RTPListener::shutdown()
{
    //notify the thread to stop
    threadRunning = false;
    NetworkWrapper::closeSocket(receiveSocket);
    receiveSocket = INVALID_SOCKET;
}


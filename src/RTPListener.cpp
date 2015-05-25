/* 
 * File:   ReceiveThread.cpp
 * Author: daniel
 * 
 * Created on May 16, 2015, 12:49 PM
 */

#include <unistd.h>

#include "RTPListener.h"

RTPListener::RTPListener(const sockaddr *receiveAddress, const RTPBuffer *buffer)
{
    this->receiveAddress = receiveAddress;
    this->buffer = buffer;
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
    receiveBuffer = (char *)malloc(RTP_HEADER_MAX_SIZE + networkConfiguration.inputBufferSize);
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
        //1. wait for package
        ssize_t receivedSize = recv(receiveSocket, receiveBuffer, networkConfiguration.inputBufferSize, MSG_DONTWAIT);
        if(receivedSize == EAGAIN || receivedSize == EWOULDBLOCK)
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        //2. read and store package
        RTPPackage *package = new RTPPackage(receivedSize);
        //TODO how to get package-data into package??
        //TODO buffer->addPackage(*package);
    }
}

void RTPListener::shutdown()
{
    //notify the thread to stop
    threadRunning = false;
    NetworkWrapper::closeSocket(receiveSocket);
    receiveSocket = INVALID_SOCKET;
}


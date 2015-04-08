/* 
 * File:   UDPWrapper.cpp
 * Author: daniel
 * 
 * Created on April 1, 2015, 10:37 AM
 */

#include "UDPWrapper.h"

using namespace std;

UDPWrapper::UDPWrapper() : NetworkWrapper()
{
}

UDPWrapper::UDPWrapper(const UDPWrapper& orig) : NetworkWrapper(orig)
{
}

UDPWrapper::~UDPWrapper()
{
}

int UDPWrapper::initializeNetwork()
{
    return NetworkWrapper::initializeNetwork();
}


int UDPWrapper::process(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
    //XXX is cast to char * possible without problems??
    long unsigned int dataSize = nFrames * NetworkWrapper::getBytesFromAudioFormat(audioConfiguration.audioFormat);
    
    //send
    long int size = sendto(NetworkWrapper::Socket, (char *)inputBuffer, dataSize, 0, &networkConfiguration.remoteAddr, sizeof(networkConfiguration.remoteAddr));
    if(size == -1)
    {
        cerr << "Error while sending UDP message: " << errno << endl;
        //cancel immediately
        return 1;
    }
    
    cout << "Sent: " << size << endl;
    
    //receive
    size = recvfrom(NetworkWrapper::Socket, (char *)outputBuffer, dataSize, 0, NULL, 0);
    if(size == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            cout << "No data available" << endl;
        }
        else
        {
            cerr << "Error while receiving UDP package: " << errno << endl;
            //cancel immediately
            return 1;
        }
    }
    
    cout << "Received: " << size << endl;
    
    //continue function
    return 0;
}

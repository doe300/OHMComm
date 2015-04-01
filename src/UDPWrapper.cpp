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
    //FIXME doesn't work yet!
    
    long unsigned int outputSize = nFrames;
    
    //send
    iovec sendData{inputBuffer, outputSize};
    const msghdr sendHeader{&networkConfiguration.remoteAddr,sizeof(networkConfiguration.remoteAddr), &sendData, outputSize};
    long int size = sendmsg(NetworkWrapper::Socket, &sendHeader, 0);
    if(size == -1)
    {
        cerr << "Error while sending UDP message: " << errno << endl;
        //cancel immediately
        return 1;
    }
    
    cout << "Sent: " << size << endl;
    
    long int inputSize = nFrames;
    
    //receive
    size = recv(NetworkWrapper::Socket, outputBuffer, inputSize, 0);
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
    //TODO need to resize received packages (or buffer)??
    
    //continue function
    return 0;
}

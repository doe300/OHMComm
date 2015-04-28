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

void UDPWrapper::configure()
{
    int errorCode = initializeNetwork();
    if(errorCode != 0)
    {
        std::cerr << "Error on network-initialization: " << errorCode << std::endl;
    }
}


int UDPWrapper::process(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
    //send
    if(inputBuffer != NULL)
    {
        long unsigned int inputBufferSize = nFrames * getBytesFromAudioFormat(audioConfiguration.InputAudioFormat);
        long int size = sendto(Socket, (char *)inputBuffer, inputBufferSize, 0, &networkConfiguration.remoteAddr, sizeof(networkConfiguration.remoteAddr));
        if(size == -1)
        {
            cerr << "Error while sending UDP message: " << errno << endl;
            //cancel immediately
            return 1;
        }
        cout << "Sent: " << size << endl;
    }
    
    //receive
    if(outputBuffer != NULL)
    {
        long unsigned int outputBufferSize = nFrames * getBytesFromAudioFormat(audioConfiguration.OutputAudioFormat);
        long int size = recvfrom(Socket, (char *)outputBuffer, outputBufferSize, 0, NULL, 0);
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
    }
    return 0;
}

/* 
 * File:   NetworkWrapper.cpp
 * Author: daniel
 * 
 * Created on April 1, 2015, 6:19 PM
 */

#include "NetworkWrapper.h"

NetworkWrapper::NetworkWrapper() : AudioProcessor(NULL)
{
    NetworkWrapper::Socket = -1;
}

NetworkWrapper::NetworkWrapper(const NetworkWrapper& orig) : AudioProcessor(orig)
{
    NetworkWrapper::Socket = -1;
}

NetworkWrapper::~NetworkWrapper()
{
    if(Socket != -1)
    {
        //close socket
         #ifdef _WIN32
        closesocket(NetworkWrapper::Socket);
        #else
        shutdown(NetworkWrapper::Socket, SHUT_RDWR);
        #endif
    }
}

int NetworkWrapper::initializeNetwork()
{
    //dummy implementation, simply initialize socket
    int result = NetworkWrapper::startWinsock();
    if(result != 0)
    {
        return result;
    }
    result = NetworkWrapper::createSocket();
    if(result == -1)
    {
        return result;
    }
    //all okay
    return 0;
}

int NetworkWrapper::startWinsock()
{
    // Starting Winsock for Windows
    #ifdef _WIN32
    WSADATA w;
    if(int result = WSAStartup(MAKEWORD(2,2), &w) != 0)
    {
        std::cerr << "Failed to start Winsock 2! Error #" << result << std::endl;
        return result;
    }
    #endif
    return 0;
}

int NetworkWrapper::createSocket()
{
    // AF_INET - creating an IPv4 based socket
    NetworkWrapper::Socket = socket(AF_INET, networkConfiguration.socketType, networkConfiguration.protocol);
    if(Socket == -1)
    {
        std::cerr << "Error on creating socket: " << errno << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Socket created for " << (networkConfiguration.protocol == IPPROTO_TCP ? "TCP" : "UDP") << std::endl;
    }
    
    //6. connect
    
    //local address
    if(bind(Socket, &networkConfiguration.localAddr, sizeof(networkConfiguration.localAddr)) == -1)
    {
        std::cerr << "Error binding the socket: " << errno << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Local port bound." << std::endl;
    }
    
    //remote address
    //for datagram-sockets, just sets default remote address
    if(connect(Socket,&networkConfiguration.remoteAddr,sizeof(networkConfiguration.remoteAddr)) == -1)
    {
        std::cerr << "Error connection the socket:" << errno << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Connection established." << std::endl;
    }
    
    return Socket;
}

uint8_t NetworkWrapper::getBytesFromAudioFormat(RtAudioFormat InputAudioFormat)
{
	switch (InputAudioFormat)
    {
        case RTAUDIO_SINT8:
            //1 byte signed integer
            return 1;
        case RTAUDIO_SINT16:
            //2 byte signed integer
            return 2;
        case RTAUDIO_SINT24:
            //3 byte signed integer
            return 3;
        case RTAUDIO_SINT32:
            //4 byte signed integer
        case RTAUDIO_FLOAT32:
            //4 byte float
            return 4;
        case RTAUDIO_FLOAT64:
            //8 byte signed integer
            return 8;
    }
    
    //TODO error-handling
    return 1;
}

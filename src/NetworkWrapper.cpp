/* 
 * File:   NetworkWrapper.cpp
 * Author: daniel
 * 
 * Created on April 1, 2015, 6:19 PM
 */

#include "NetworkWrapper.h"

NetworkWrapper::NetworkWrapper() : AudioProcessor(NULL)
{
}

NetworkWrapper::NetworkWrapper(const NetworkWrapper& orig) : AudioProcessor(orig)
{
}

NetworkWrapper::~NetworkWrapper()
{
    if(Socket != -1)
    {
        //close socket
         #ifdef __linux__
        shutdown(NetworkWrapper::Socket, SHUT_RDWR);
        #else
        closesocket(NetworkWrapper::Socket);
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
    #ifndef __linux__
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




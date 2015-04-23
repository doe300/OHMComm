/* 
 * File:   NetworkWrapper.cpp
 * Author: daniel
 * 
 * Created on April 1, 2015, 6:19 PM
 */

#include "NetworkWrapper.h"

NetworkWrapper::NetworkWrapper() : AudioProcessor()
{
    Socket = INVALID_SOCKET;
}

NetworkWrapper::NetworkWrapper(const NetworkWrapper& orig) : AudioProcessor(orig)
{
    Socket = INVALID_SOCKET;
}

NetworkWrapper::~NetworkWrapper()
{
    if(Socket != INVALID_SOCKET)
    {
        //close socket
         #ifdef _WIN32
        closesocket(Socket);
        WSACleanup();
        #else
        shutdown(Socket, SHUT_RDWR);
        #endif
    }
}

int NetworkWrapper::initializeNetwork()
{
    //dummy implementation, simply initialize socket
    int result = startWinsock();
    if(result != 0)
    {
        return result;
    }
    result = createSocket();
    if(result == INVALID_SOCKET)
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
    Socket = socket(AF_INET, networkConfiguration.socketType, networkConfiguration.protocol);
    if(Socket == INVALID_SOCKET)
    {
        std::cerr << "Error on creating socket: " << getLastError() << std::endl;
        return INVALID_SOCKET;
    }
    else
    {
        std::cout << "Socket created for " << (networkConfiguration.protocol == IPPROTO_TCP ? "TCP" : "UDP") << std::endl;
    }
    
    //6. connect
    
    //local address
    if(bind(Socket, &networkConfiguration.localAddr, sizeof(networkConfiguration.localAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding the socket: " << getLastError() << std::endl;
        return SOCKET_ERROR;
    }
    else
    {
        std::cout << "Local port bound." << std::endl;
    }
    
    //remote address
    //for datagram-sockets, just sets default remote address
    if(connect(Socket,&networkConfiguration.remoteAddr,sizeof(networkConfiguration.remoteAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error connection the socket:" << getLastError() << std::endl;
        return SOCKET_ERROR;
    }
    else
    {
        std::cout << "Connection established." << std::endl;
    }
    
    return Socket;
}

uint8_t NetworkWrapper::getBytesFromAudioFormat(RtAudioFormat audioFormat)
{
	switch (audioFormat)
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

int NetworkWrapper::getLastError()
{
	int error;
	#ifdef _WIN32
		error = WSAGetLastError();
	#else
		error = errno;
	#endif

	return error;
}
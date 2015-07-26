#include "UDPWrapper.h"

UDPWrapper::UDPWrapper(unsigned short portIncoming, std::string remoteIPAddress, unsigned short portOutgoing) :
    localAddress({0}), remoteAddress({0})
{
	InitializeNetworkConfig(portIncoming, remoteIPAddress, portOutgoing);
	initializeNetwork();
}

UDPWrapper::UDPWrapper(struct NetworkConfiguration networkConfig) : 
    UDPWrapper(networkConfig.portIncoming, networkConfig.addressOutgoing, networkConfig.portOutgoing) 
{
}

UDPWrapper::~UDPWrapper()
{
    if(Socket >= 0)
    {
        closeNetwork();
    }
}


void UDPWrapper::initializeNetwork()
{
	startWinsock();
	createSocket();
}

void UDPWrapper::startWinsock()
{
	// Starting Winsock for Windows
	#ifdef _WIN32
	WSADATA w;
	if (int result = WSAStartup(MAKEWORD(2, 2), &w) != 0)
	{
		std::cerr << "Failed to start Winsock 2! Error #" << result << std::endl;
	}
	#endif
}

void UDPWrapper::InitializeNetworkConfig(unsigned short localPort, std::string remoteAddress, unsigned short remotePort)
{
    if(NetworkWrapper::isIPv6(remoteAddress))
    {
        sockaddr_in6* localAdr = reinterpret_cast<sockaddr_in6*>(&(this->localAddress));
        sockaddr_in6* remoteAdr = reinterpret_cast<sockaddr_in6*>(&(this->remoteAddress));
        localAdr->sin6_family = AF_INET6;
        //listen on any address of this computer (localhost, local IP, ...)
        localAdr->sin6_addr = in6addr_any;
        localAdr->sin6_port = htons(localPort);
        remoteAdr->sin6_family = AF_INET6;
        inet_pton(AF_INET6, remoteAddress.c_str(), &(remoteAdr->sin6_addr));
        remoteAdr->sin6_port = htons(remotePort);
    }
    else
    {
        sockaddr_in* localAdr = reinterpret_cast<sockaddr_in*>(&(this->localAddress));
        sockaddr_in* remoteAdr = reinterpret_cast<sockaddr_in*>(&(this->remoteAddress));
        localAdr->sin_family = AF_INET;
        //listen on any address of this computer (localhost, local IP, ...)
        localAdr->sin_addr.s_addr = INADDR_ANY;
        localAdr->sin_port = htons(localPort);
        remoteAdr->sin_family = AF_INET;
        inet_pton(AF_INET, remoteAddress.c_str(), &(remoteAdr->sin_addr));
        remoteAdr->sin_port = htons(remotePort);
    }
}


void UDPWrapper::createSocket()
{
    //TODO make dependable on IPv4/IPv6
	// AF_INET - creating an IPv4 based socket
        // AF_INET6 - creating an IPv6 based socket
	this->Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (Socket == INVALID_SOCKET)
	{
		std::cerr << "Error on creating socket: " << getLastError() << std::endl;
		return;
	}
	else
	{
		std::cout << "Socket created." << std::endl;
	}


	if (bind(Socket, &(this->localAddress), sizeof(localAddress)) == SOCKET_ERROR)
	{
		std::cerr << "Error binding the socket: " << getLastError() << std::endl;
		return;
	}
	else
	{
		std::cout << "Local port bound." << std::endl;
	}
}

int UDPWrapper::sendDataNetworkWrapper(void *buffer, unsigned int bufferSize)
{
	return sendto(this->Socket, (char*)buffer, (int)bufferSize, 0, &(this->remoteAddress), sizeof(remoteAddress));
}

int UDPWrapper::recvDataNetworkWrapper(void *buffer, unsigned int bufferSize)
{
	unsigned int localAddrLen = sizeof(localAddress);
	int result = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, &(this->localAddress), &localAddrLen);
	if (result == -1)
		std::cerr << this->getLastError();
        return result;
}


int UDPWrapper::getLastError()
{
    int error;
    #ifdef _WIN32
    error = WSAGetLastError();
    #else
    error = errno;
    #endif
    return error;
}

void UDPWrapper::closeNetwork()
{
    shutdown(Socket, SHUTDOWN_BOTH);
    #ifdef _WIN32
    closesocket(Socket);
    #else
    close(Socket);
    #endif
    Socket = INVALID_SOCKET;
}

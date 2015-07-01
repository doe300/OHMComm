#include "UDPWrapper.h"

UDPWrapper::UDPWrapper(sockaddr_in localAddress, sockaddr_in remoteAddress) : localAddress(localAddress), remoteAddress(remoteAddress)
{}

UDPWrapper::UDPWrapper(unsigned short portIncoming, std::string remoteIPAddress, unsigned short portOutgoing)
{
	InitializeNetworkConfig(portIncoming, remoteIPAddress, portOutgoing);
	initializeNetwork();
}

UDPWrapper::UDPWrapper(struct NetworkConfiguration networkConfig) : 
    UDPWrapper(networkConfig.portIncoming, networkConfig.addressOutgoing, networkConfig.portOutgoing) 
{}

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
	this->localAddress.sin_family = AF_INET;
	this->localAddress.sin_port = htons(localPort);
        //listen on any address of this computer (localhost, local IP, ...)
        this->localAddress.sin_addr.s_addr = INADDR_ANY;

	this->remoteAddress.sin_family = AF_INET;
	this->remoteAddress.sin_port = htons(remotePort);
	unsigned long addr2 = inet_addr(remoteAddress.c_str());
	memcpy((char *)&this->remoteAddress.sin_addr, &addr2, sizeof(addr2));
}


void UDPWrapper::createSocket()
{
	// AF_INET - creating an IPv4 based socket
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


	if (bind(Socket, (sockaddr*)&this->localAddress, sizeof(localAddress)) == SOCKET_ERROR)
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
	return sendto(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&this->remoteAddress, sizeof(remoteAddress));
}

int UDPWrapper::recvDataNetworkWrapper(void *buffer, unsigned int bufferSize)
{
	#ifdef _WIN32
	int localAddrLen = sizeof(localAddress);
	#else
	socklen_t localAddrLen = sizeof(localAddress);
	#endif
	int result = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&this->localAddress, &localAddrLen);
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

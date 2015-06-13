#include "UDPWrapper.h"

UDPWrapper::UDPWrapper(sockaddr_in localAddress, sockaddr_in remoteAddress, unsigned int outputBufferSize, unsigned int inputBufferSize) :
localAddress(localAddress), remoteAddress(remoteAddress), outputBufferSize(outputBufferSize), inputBufferSize(inputBufferSize)
{}

UDPWrapper::UDPWrapper(std::string localIPAddress, unsigned short portIncoming, std::string remoteIPAddress, unsigned short portOutgoing, unsigned int outputBufferSize, unsigned int inputBufferSize) :
outputBufferSize(outputBufferSize), inputBufferSize(inputBufferSize)
{
	InitializeNetworkConfig(localIPAddress, portIncoming, remoteIPAddress, portOutgoing);
	initializeNetwork();
}

UDPWrapper::UDPWrapper(struct NetworkConfiguration networkConfig) : 
UDPWrapper(networkConfig.addressIncoming, networkConfig.portIncoming, networkConfig.addressOutgoing, 
networkConfig.portOutgoing, networkConfig.outputBufferSize, networkConfig.inputBufferSize) {}

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

void UDPWrapper::InitializeNetworkConfig(std::string addressIncoming, unsigned short portIncoming, std::string addressOutgoing, unsigned short portOutgoing)
{
	this->localAddress.sin_family = AF_INET;
	this->localAddress.sin_port = htons(portIncoming);
	unsigned long addr1 = inet_addr(addressIncoming.c_str());
	memcpy((char *)&this->localAddress.sin_addr, &addr1, sizeof(addr1));

	this->remoteAddress.sin_family = AF_INET;
	this->remoteAddress.sin_port = htons(portOutgoing);
	unsigned long addr2 = inet_addr(addressOutgoing.c_str());
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

void UDPWrapper::closeSocket(int fd)
{
    shutdown(fd, SHUTDOWN_BOTH);
    #ifdef _WIN32
    closesocket(fd);
    #else
    close(fd);
    #endif
}

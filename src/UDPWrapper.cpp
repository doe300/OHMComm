#include "UDPWrapper.h"

UDPWrapper::UDPWrapper(sockaddr_in addressDataIncoming, sockaddr_in addressDataOutgoing, unsigned int outputBufferSize, unsigned int inputBufferSize) :
addressDataIncoming(addressDataIncoming), addressDataOutgoing(addressDataOutgoing), outputBufferSize(outputBufferSize), inputBufferSize(inputBufferSize)
{}

UDPWrapper::UDPWrapper(std::string addressIncoming, unsigned short portIncoming, std::string addressOutgoing, unsigned short portOutgoing, unsigned int outputBufferSize, unsigned int inputBufferSize) :
outputBufferSize(outputBufferSize), inputBufferSize(inputBufferSize)
{
	InitializeNetworkConfig(addressIncoming, portIncoming, addressOutgoing, portOutgoing);
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
	this->addressDataIncoming.sin_family = AF_INET;
	this->addressDataIncoming.sin_port = htons(portIncoming);
	unsigned long addr1 = inet_addr(addressIncoming.c_str());
	memcpy((char *)&this->addressDataIncoming.sin_addr, &addr1, sizeof(addr1));

	this->addressDataOutgoing.sin_family = AF_INET;
	this->addressDataOutgoing.sin_port = htons(portOutgoing);
	unsigned long addr2 = inet_addr(addressOutgoing.c_str());
	memcpy((char *)&this->addressDataOutgoing.sin_addr, &addr2, sizeof(addr2));
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


	if (bind(Socket, (sockaddr*)&this->addressDataIncoming, sizeof(addressDataIncoming)) == SOCKET_ERROR)
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
	return sendto(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&this->addressDataOutgoing, sizeof(addressDataOutgoing));
}

int UDPWrapper::recvDataNetworkWrapper(void *buffer, unsigned int bufferSize)
{
	#ifdef _WIN32
	int remoteAddrLen = sizeof(addressDataIncoming);
	#else
	socklen_t remoteAddrLen = sizeof(addressDataIncoming);
	#endif
	int result = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&this->addressDataIncoming, &remoteAddrLen);
	if (result == -1)
		std::cerr << this->getLastError();
        return result;
}


#include "UDPWrapper.h"

UDPWrapper::UDPWrapper(unsigned short portIncoming, const std::string remoteIPAddress, unsigned short portOutgoing) :
    localAddress({0}), remoteAddress({0})
{
	initializeNetworkConfig(portIncoming, remoteIPAddress, portOutgoing);
	initializeNetwork();
}

UDPWrapper::UDPWrapper(const NetworkConfiguration& networkConfig) :
    UDPWrapper(networkConfig.localPort, networkConfig.remoteIPAddress, networkConfig.remotePort)
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

void UDPWrapper::initializeNetworkConfig(unsigned short localPort, const std::string remoteAddress, unsigned short remotePort)
{
    if(NetworkWrapper::isIPv6(remoteAddress))
    {
        std::cout << "Using IPv6 ..." << std::endl;
        isIPv6 = true;
        sockaddr_in6* localAdr = reinterpret_cast<sockaddr_in6*>(&(this->localAddress));
        sockaddr_in6* remoteAdr = reinterpret_cast<sockaddr_in6*>(&(this->remoteAddress));
        localAdr->sin6_family = AF_INET6;
        //listen on any address of this computer (localhost, local IP, ...)
        localAdr->sin6_addr = in6addr_any;
        localAdr->sin6_port = htons(localPort);
        localAdr->sin6_flowinfo = {0};
        localAdr->sin6_scope_id = {0};
        remoteAdr->sin6_family = AF_INET6;
        inet_pton(AF_INET6, remoteAddress.c_str(), &(remoteAdr->sin6_addr));
        remoteAdr->sin6_port = htons(remotePort);
    }
    else
    {
        std::cout << "Using IPv4 ..." << std::endl;
        isIPv6 = false;
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


bool UDPWrapper::createSocket()
{
    unsigned int addressLength = getSocketAddressLength();
    // AF_INET - creating an IPv4 based socket
    // AF_INET6 - creating an IPv6 based socket
    if(isIPv6)
    {
        this->Socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        this->Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (Socket == INVALID_SOCKET)
    {
        std::wcerr << "Error on creating socket: " << getLastError() << std::endl;
        return false;
    }
    else
    {
        std::cout << "Socket created." << std::endl;
    }
    
    //FIXME bind doesn't work yet for IPv6 -- EADDRNOTAVAIL
    if (bind(Socket, (sockaddr*)&(this->localAddress), addressLength) == SOCKET_ERROR)
    {
        std::wcerr << "Error binding the socket: " << getLastError() << std::endl;
        return false;
    }
    else
    {
        std::cout << "Local port bound." << std::endl;
    }
    return true;
}

int UDPWrapper::sendData(void *buffer, unsigned int bufferSize)
{
	#if TESTMODE
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 100);
	int i = dis(gen);
	if (i <= PACKET_LOSS_CHANCE_IN_PERCENT)
		return bufferSize;

	#endif
    return sendto(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(this->remoteAddress), getSocketAddressLength());

}

int UDPWrapper::receiveData(void *buffer, unsigned int bufferSize)
{
    #ifdef _WIN32
    int localAddrLen = sizeof(localAddress);
    #else
    unsigned int localAddrLen = getSocketAddressLength();
    #endif
    int result = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(this->localAddress), &localAddrLen);
    if (result == -1)
        std::wcerr << this->getLastError();
    return result;
}


std::wstring UDPWrapper::getLastError() const
{
    int error;
    #ifdef _WIN32
    error = WSAGetLastError();
	wchar_t* tmp;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,0, error, LANG_USER_DEFAULT, (wchar_t*)&tmp, 0, nullptr);
    #else
    error = errno;
    wchar_t tmp[255];
    char* errPtr = strerror(error);
    mbstowcs(tmp, errPtr, 255);
    #endif
    return (std::to_wstring(error) + L" - ") + tmp;
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

const int UDPWrapper::getSocketAddressLength()
{
    if(isIPv6)
    {
        return sizeof(sockaddr_in6);
    }
    return sizeof(sockaddr_in);
}

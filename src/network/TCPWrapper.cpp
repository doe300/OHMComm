#include "Logger.h"
#include "network/TCPWrapper.h"
#include "network/NetworkGrammars.h"

using namespace ohmcomm::network;

TCPWrapper::TCPWrapper(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort) :
    localAddress({0}), remoteAddress({0})
{
	initializeNetworkConfig(localPort, remoteIPAddress, remotePort);
	initializeNetwork();
}

TCPWrapper::TCPWrapper(const ohmcomm::NetworkConfiguration& networkConfig) :
    TCPWrapper(networkConfig.localPort, networkConfig.remoteIPAddress, networkConfig.remotePort)
{
}

TCPWrapper::~TCPWrapper()
{
    if(Socket >= 0)
    {
        closeNetwork();
    }
}


void TCPWrapper::initializeNetwork()
{
	startWinsock();
	createSocket();
}

void TCPWrapper::startWinsock()
{
	// Starting Winsock for Windows
	#ifdef _WIN32
	WSADATA w;
	if (int result = WSAStartup(MAKEWORD(2, 2), &w) != 0)
	{
		ohmcomm::error("TCP") << "Failed to start Winsock 2! Error #" << result << ohmcomm::endl;
	}
	#endif
}

void TCPWrapper::initializeNetworkConfig(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort)
{
    if(NetworkGrammars::isIPv6Address(remoteIPAddress))
    {
        ohmcomm::info("TCP") << "Using IPv6 ..." << ohmcomm::endl;
        localAddress = SocketAddress::createLocalAddress(true, localPort);
    }
    else
    {
        ohmcomm::info("TCP") << "Using IPv4 ..." << ohmcomm::endl;
        localAddress = SocketAddress::createLocalAddress(false, localPort);
    }
    remoteAddress = SocketAddress::fromAddressAndPort(remoteIPAddress, remotePort);
}

bool TCPWrapper::createSocket()
{
    unsigned int addressLength = getSocketAddressLength();
    // AF_INET - creating an IPv4 based socket
    // AF_INET6 - creating an IPv6 based socket
    if(remoteAddress.isIPv6)
    {
        this->Socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        this->Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    if (Socket == INVALID_SOCKET)
    {
        ohmcomm::error("TCP") << "Error on creating socket: " << getLastError() << ohmcomm::endl;
        return false;
    }
    else
    {
        ohmcomm::info("TCP") << "Socket created." << ohmcomm::endl;
    }
    
    //set socket timeout to 1sec
#ifdef _WIN32
    DWORD timeout = 1000;
#else
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
#endif
    int yes = 1;
    setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    //we need to allow reuse-address for fast re-binding after closing
    setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof (int));
    
    if (bind(Socket, (sockaddr*)&(this->localAddress), addressLength) == SOCKET_ERROR)
    {
        ohmcomm::error("TCP") << "Error binding the socket: " << getLastError() << ohmcomm::endl;
        return false;
    }
    else
    {
        ohmcomm::info("TCP") << "Local port bound." << ohmcomm::endl;
    }
    
    if(connect(Socket, (sockaddr*)&(this->remoteAddress), addressLength) == SOCKET_ERROR)
    {
        ohmcomm::error("TCP") << "Error connecting the socket: " << getLastError() << ohmcomm::endl;
        return false;
    }
    else
    {
        ohmcomm::info("TCP") << "Connection established." << ohmcomm::endl;
    }
    return true;
}

int TCPWrapper::sendData(const void *buffer, const unsigned int bufferSize)
{
    return sendto(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(this->remoteAddress), getSocketAddressLength());
}

NetworkWrapper::Package TCPWrapper::receiveData(void *buffer, unsigned int bufferSize)
{
#ifdef _WIN32
    int addressLength = getSocketAddressLength();
#else
    unsigned int addressLength = getSocketAddressLength();
#endif
    NetworkWrapper::Package package{};
    package.status = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(package.address.ipv6), &addressLength);
    if (package.status == -1)
    {
        if(hasTimedOut() || errno == INTERRUPTED_BY_SYSTEM_CALL)
        {
            //we have timed-out (or were interrupted by some other system call), so notify caller and return
            package.status = RECEIVE_TIMEOUT;
            return package;
        }
        std::wcerr << this->getLastError();
    }
    if(addressLength == sizeof(sockaddr_in6))
        package.address.isIPv6 = true;
    return package;
}

void TCPWrapper::closeNetwork()
{
    if(Socket != INVALID_SOCKET)
    {
        shutdown(Socket, SHUTDOWN_BOTH);
        #ifdef _WIN32
        closesocket(Socket);
        #else
        close(Socket);
        #endif
        Socket = INVALID_SOCKET;
    }
}

int TCPWrapper::getSocketAddressLength()
{
    if(remoteAddress.isIPv6)
    {
        return sizeof(sockaddr_in6);
    }
    return sizeof(sockaddr_in);
}

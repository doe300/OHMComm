#include "network/TCPWrapper.h"

using namespace ohmcomm;

TCPWrapper::TCPWrapper(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort) :
    localAddress({0}), remoteAddress({0})
{
	initializeNetworkConfig(localPort, remoteIPAddress, remotePort);
	initializeNetwork();
}

TCPWrapper::TCPWrapper(const NetworkConfiguration& networkConfig) :
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
		std::cerr << "Failed to start Winsock 2! Error #" << result << std::endl;
	}
	#endif
}

void TCPWrapper::initializeNetworkConfig(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort)
{
    if(NetworkWrapper::isIPv6(remoteIPAddress))
    {
        std::cout << "Using IPv6 ..." << std::endl;
        isIPv6 = true;
        localAddress.ipv6.sin6_family = AF_INET6;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv6.sin6_addr = in6addr_any;
        localAddress.ipv6.sin6_port = htons(localPort);
        localAddress.ipv6.sin6_flowinfo = {0};
        localAddress.ipv6.sin6_scope_id = {0};
        remoteAddress.ipv6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, remoteIPAddress.c_str(), &(remoteAddress.ipv6.sin6_addr));
        remoteAddress.ipv6.sin6_port = htons(remotePort);
    }
    else
    {
        std::cout << "Using IPv4 ..." << std::endl;
        isIPv6 = false;
        localAddress.ipv4.sin_family = AF_INET;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv4.sin_addr.s_addr = INADDR_ANY;
        localAddress.ipv4.sin_port = htons(localPort);
        remoteAddress.ipv4.sin_family = AF_INET;
        inet_pton(AF_INET, remoteIPAddress.c_str(), &(remoteAddress.ipv4.sin_addr));
        remoteAddress.ipv4.sin_port = htons(remotePort);
    }
}


bool TCPWrapper::createSocket()
{
    unsigned int addressLength = getSocketAddressLength();
    // AF_INET - creating an IPv4 based socket
    // AF_INET6 - creating an IPv6 based socket
    if(isIPv6)
    {
        this->Socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        this->Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
        std::wcerr << "Error binding the socket: " << getLastError() << std::endl;
        return false;
    }
    else
    {
        std::cout << "Local port bound." << std::endl;
    }
    
    if(connect(Socket, (sockaddr*)&(this->remoteAddress), addressLength) == SOCKET_ERROR)
    {
        std::wcerr << "Error connecting the socket: " << getLastError() << std::endl;
        return false;
    }
    else
    {
        std::cout << "Connection established." << std::endl;
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
    package.status = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(package.ipv6Address), &addressLength);
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
        package.isIPv6 = true;
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
    if(isIPv6)
    {
        return sizeof(sockaddr_in6);
    }
    return sizeof(sockaddr_in);
}

#include "network/UDPWrapper.h"

using namespace ohmcomm::network;

UDPWrapper::UDPWrapper(unsigned short portIncoming, const std::string remoteIPAddress, unsigned short portOutgoing) :
localAddress({0}), remoteAddress({0})
{
    initializeNetworkConfig(portIncoming, remoteIPAddress, portOutgoing);
    initializeNetwork();
}

UDPWrapper::UDPWrapper(const ohmcomm::NetworkConfiguration& networkConfig) :
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

void UDPWrapper::initializeNetworkConfig(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort)
{
    if(NetworkWrapper::isIPv6(remoteIPAddress))
    {
        std::cout << "Using IPv6 ..." << std::endl;
        localAddress.ipv6.sin6_family = AF_INET6;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv6.sin6_addr = in6addr_any;
        localAddress.ipv6.sin6_port = htons(localPort);
        localAddress.ipv6.sin6_flowinfo = {0};
        localAddress.ipv6.sin6_scope_id = {0};
        remoteAddress.ipv6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, remoteIPAddress.c_str(), &(remoteAddress.ipv6.sin6_addr));
        remoteAddress.ipv6.sin6_port = htons(remotePort);
        remoteAddress.isIPv6 = true;
    }
    else
    {
        std::cout << "Using IPv4 ..." << std::endl;
        localAddress.ipv4.sin_family = AF_INET;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv4.sin_addr.s_addr = INADDR_ANY;
        localAddress.ipv4.sin_port = htons(localPort);
        remoteAddress.ipv4.sin_family = AF_INET;
        inet_pton(AF_INET, remoteIPAddress.c_str(), &(remoteAddress.ipv4.sin_addr));
        remoteAddress.ipv4.sin_port = htons(remotePort);
        remoteAddress.isIPv6 = false;
    }
}

bool UDPWrapper::createSocket()
{
    unsigned int addressLength = getSocketAddressLength();
    // AF_INET - creating an IPv4 based socket
    // AF_INET6 - creating an IPv6 based socket
    if(remoteAddress.isIPv6)
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
    
    //set socket timeout to 1sec
#ifdef _WIN32
    DWORD timeout = 1000;
#else
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
#endif
    int yes = 1;
    setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof (timeout));
    //we need to allow reuse-address for fast re-binding after closing
    if(setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof (int)) < 0)
    {
        perror("setsockopt");
        std::wcout << "Reuse: " << getLastError() << std::endl;
    }

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

int UDPWrapper::sendData(const void *buffer, const unsigned int bufferSize)
{
    return sendto(this->Socket, (char*) buffer, (int) bufferSize, 0, (sockaddr*)&(this->remoteAddress), getSocketAddressLength());
}

NetworkWrapper::Package UDPWrapper::receiveData(void *buffer, unsigned int bufferSize)
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

void UDPWrapper::closeNetwork()
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

int UDPWrapper::getSocketAddressLength()
{
    if(remoteAddress.isIPv6)
    {
        return sizeof(sockaddr_in6);
    }
    return sizeof (sockaddr_in);
}

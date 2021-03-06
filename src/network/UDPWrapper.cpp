#include "Logger.h"
#include "network/UDPWrapper.h"
#include "network/NetworkGrammars.h"

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
        ohmcomm::error("UDP") << "Failed to start Winsock 2! Error #" << result << ohmcomm::endl;
    }
#endif
}

void UDPWrapper::initializeNetworkConfig(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort)
{
    if(NetworkGrammars::isIPv6Address(remoteIPAddress))
    {
        ohmcomm::info("UDP") << "Using IPv6 ..." << ohmcomm::endl;
        localAddress = SocketAddress::createLocalAddress(true, localPort);
    }
    else
    {
        ohmcomm::info("UDP") << "Using IPv4 ..." << ohmcomm::endl;
        localAddress = SocketAddress::createLocalAddress(false, localPort);
    }
    remoteAddress = SocketAddress::fromAddressAndPort(remoteIPAddress, remotePort);
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
        ohmcomm::error("UDP") << "Error on creating socket: " << getLastError() << ohmcomm::endl;
        return false;
    }
    else
    {
        ohmcomm::info("UDP") << "Socket created." << ohmcomm::endl;
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
        ohmcomm::info("UDP") << "Reuse: " << getLastError() << ohmcomm::endl;
    }

    if (bind(Socket, (sockaddr*)&(this->localAddress), addressLength) == SOCKET_ERROR)
    {
        ohmcomm::error("UDP") << "Error binding the socket: " << getLastError() << ohmcomm::endl;
        return false;
    }
    else
    {
        ohmcomm::info("UDP") << "Local port bound." << ohmcomm::endl;
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

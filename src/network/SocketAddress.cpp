
#include "network/SocketAddress.h"
#include "network/NetworkGrammars.h"
#include "Utility.h"

using namespace ohmcomm::network;

std::pair<std::string, uint16_t> SocketAddress::toAddressAndPort() const
{
    uint16_t port;
    char buffer[65];
    if (isIPv6) {
        inet_ntop(ipv6.sin6_family, &(ipv6.sin6_addr), buffer, 64);
        port = ntohs(ipv6.sin6_port);
    }
    else {
        inet_ntop(ipv4.sin_family, &(ipv4.sin_addr), buffer, 64);
        port = ntohs(ipv4.sin_port);
    }
    return std::make_pair(std::string(buffer), port);
}

SocketAddress SocketAddress::fromAddressAndPort(const std::string& address, const uint16_t port)
{
    SocketAddress addr;
    std::string ipAddress;
    if(NetworkGrammars::isIPv6Address(address))
    {
        addr.isIPv6 = true;
        ipAddress = address;
    }
    else if(NetworkGrammars::isIPv4Address(address))
    {
        ipAddress = address;
    }
    else if(NetworkGrammars::isValidDomainName(address))
    {
        ipAddress = ohmcomm::Utility::getAddressForHostName(address);
        if(ipAddress.empty())
        {
            return {{},false};
        }
        if(NetworkGrammars::isIPv6Address(ipAddress))
        {
            addr.isIPv6 = true;
        }
    }
    
    if(addr.isIPv6)
    {
        addr.ipv6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, ipAddress.c_str(), &(addr.ipv6.sin6_addr));
        addr.ipv6.sin6_port = htons(port);
    }
    else
    {
        addr.ipv4.sin_family = AF_INET;
        inet_pton(AF_INET, ipAddress.c_str(), &(addr.ipv4.sin_addr));
        addr.ipv4.sin_port = htons(port);
    }
    return addr;
}

SocketAddress SocketAddress::createLocalAddress(const bool isIPv6, const uint16_t localPort)
{
    SocketAddress localAddress;
    if(isIPv6)
    {
        localAddress.ipv6.sin6_family = AF_INET6;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv6.sin6_addr = in6addr_any;
        localAddress.ipv6.sin6_port = htons(localPort);
        localAddress.ipv6.sin6_flowinfo = {0};
        localAddress.ipv6.sin6_scope_id = {0};
    }
    else
    {
        localAddress.ipv4.sin_family = AF_INET;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv4.sin_addr.s_addr = INADDR_ANY;
        localAddress.ipv4.sin_port = htons(localPort);
    }
    return localAddress;
}

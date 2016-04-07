/* 
 * File:   MulticastNetworkWrapper.cpp
 * Author: daniel
 * 
 * Created on February 27, 2016, 11:47 AM
 */
#include <string.h>

#include "network/MulticastNetworkWrapper.h"
#include "network/UDPWrapper.h"

using namespace ohmcomm::network;

MulticastNetworkWrapper::MulticastNetworkWrapper(const ohmcomm::NetworkConfiguration& initialDestination) : 
    UDPWrapper(initialDestination), destinations()
{
    //add default remote-address to list of destinations
    destinations.push_back(remoteAddress);
}

int MulticastNetworkWrapper::sendData(const void* buffer, const unsigned int bufferSize)
{
    int totalBytesSent = 0;
    for(const SocketAddress& dest: destinations)
    {
        const int socketAddressLength = dest.isIPv6 ? sizeof(sockaddr_in6) : sizeof (sockaddr_in);
        const int status = sendto(this->Socket, (char*) buffer, (int) bufferSize, 0, (sockaddr*)&(dest.ipv6), socketAddressLength);
        if(status < 0)
        {
            std::wcout << "Error sending: " << getLastError() << std::endl;
        }
        totalBytesSent += status;
    }
    return totalBytesSent/destinations.size();
}

bool MulticastNetworkWrapper::addDestination(const std::string& destinationAddress, const unsigned short destinationPort)
{
    const SocketAddress tmp = SocketAddress::fromAddressAndPort(destinationAddress, destinationPort);
    for(const SocketAddress& addr : destinations)
    {
        if(tmp.isIPv6 != addr.isIPv6)
        {
            //different IP versions - cannot match
            continue;
        }
        else if(tmp.isIPv6)
        {
            if(memcmp(&(tmp.ipv6.sin6_addr), &(addr.ipv6.sin6_addr), sizeof(tmp.ipv6.sin6_addr)) == 0 && tmp.ipv6.sin6_port == addr.ipv6.sin6_port)
            {
                //already in list
                return false;
            }
        }
        else
        {
            if(memcmp(&(tmp.ipv4.sin_addr), &(addr.ipv4.sin_addr), sizeof(tmp.ipv4.sin_addr)) == 0 && tmp.ipv4.sin_port == addr.ipv4.sin_port)
            {
                //already in list
                return false;
            }
        }
    }
    destinations.push_back(tmp);
    return true;
}

bool MulticastNetworkWrapper::removeDestination(const std::string& destinationAddress, const unsigned short destinationPort)
{
    SocketAddress tmp = SocketAddress::fromAddressAndPort(destinationAddress, destinationPort);
    for(auto i = destinations.begin(); i < destinations.end(); ++i)
    {
        if(tmp.isIPv6 != (*i).isIPv6)
        {
            //different IP versions - cannot match
            continue;
        }
        else if(tmp.isIPv6)
        {
            if(memcmp(&(tmp.ipv6.sin6_addr), &((*i).ipv6.sin6_addr), sizeof(tmp.ipv6.sin6_addr)) == 0 && tmp.ipv6.sin6_port == (*i).ipv6.sin6_port)
            {
                destinations.erase(i);
                return true;
            }
        }
        else
        {
            if(memcmp(&(tmp.ipv4.sin_addr), &((*i).ipv4.sin_addr), sizeof(tmp.ipv4.sin_addr)) == 0 && tmp.ipv4.sin_port == (*i).ipv4.sin_port)
            {
                destinations.erase(i);
                return true;
            }
        }
    }
    return false;
}
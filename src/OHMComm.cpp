/* 
 * File:   OHMComm.cpp
 * Author: daniel
 *
 * Created on March 29, 2015, 1:49 PM
 */

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#ifdef __linux__
#include <arpa/inet.h> // sockaddr_in
#else
#include <winsock2.h>
#endif

#include "configuration.h"

//Declare Configurations
NetworkConfiguration networkConfiguration;
AudioConfiguration audioConfiguration;

using namespace std;

//conversion method
//returns -1 on error
inline int convertToInt(const std::string& s)
{
    std::istringstream i(s);
    int x;
    if (!(i >> x))
        return -1;
    return x;
}

inline void createAddress(sockaddr *addr, int addressType, std::string ipString, int port)
{
    sockaddr_in *address = reinterpret_cast<sockaddr_in*>(addr);
    //IPv4 or IPv6
    address->sin_family = addressType;
    
    if(ipString == "")
    {
        //listen on any address
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        //the remote IP address - inet_addr() is a convenience-method to convert from aaa.bbb.ccc.ddd (octal representation) to a 4-byte unsigned long
        address->sin_addr.s_addr = inet_addr(ipString.c_str());
    }
    //network byte order is big-endian, so we must convert the port-number
    address->sin_port = htons(port);
}

void configureNetwork()
{
    string ipString;
    string destPortString, localPortString;
    string protocolString;
    
    //1. remote address
    cout << "Input destination IP address: ";
    cin >> ipString;
    
    //2. remote and local ports
    cout << "Input destination port: ";
    cin >> destPortString;
    cout << "Input local port: ";
    cin >> localPortString;
    
    //3. protocol
    cout << "Choose protocol (TCP/UDP) [defaults to UDP]: ";
    cin >> protocolString;
    
    //4. parse arguments
    int destPort = convertToInt(destPortString);
    int localPort = convertToInt(localPortString);
    
    if(protocolString == "TCP")
    {
        // SOCK_STREAM - creating a stream-socket
        // IPPROTO_TCP - use TCP/IP protocol
        networkConfiguration.socketType = SOCK_STREAM;
        networkConfiguration.protocol = IPPROTO_TCP;
    }
    else
    {
        // SOCK_DGRAM - creating a datagram-socket
        // IPPROTO_UDP - use UDP protocol
        networkConfiguration.socketType = SOCK_DGRAM;
        networkConfiguration.protocol = IPPROTO_UDP;
    }
    
    //5. create addresses
    //local address
    createAddress(&networkConfiguration.localAddr, AF_INET, "", localPort);
    //remote address
    createAddress(&networkConfiguration.remoteAddr, AF_INET, ipString, destPort);
}

/*
 * 
 */
int main(int argc, char** argv)
{
    ////
    // Configuration
    ////
    
    //1. network connection
    configureNetwork();
    
    //2. audio devices
    //2.1 audio input
    //2.2 audio output
    //2.3 audio configuration (bit rate, ...)
    
    //3. processors
    //3.1 filters
    //3.2 codecs
    //3.3 compressors
    
    ////
    // Initialize
    ////
    
    //1. RTP
    //2. AudioProcessors
    //3. RTAudio
    
    ////
    // Running
    ////
    
    //start loop
    
    return 0;
}


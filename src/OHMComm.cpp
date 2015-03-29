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
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h>
#include <stdexcept> // sockaddr_in
#else
#include <winsock2.h>
#endif

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

inline sockaddr* createAddress(int addressType, std::string ipString, int port)
{
    //we need to create the address on the heap, because it is used outside of its scope
    sockaddr_in *address = new sockaddr_in;
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
    
    //we must cast sockaddr_in to a sockaddr struct
    return reinterpret_cast<sockaddr*>(address);
}

int configureNetwork()
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
    int socketType, protocol;
    
    if(protocolString == "TCP")
    {
        // SOCK_STREAM - creating a stream-socket
        // IPPROTO_TCP - use TCP/IP protocol
        socketType = SOCK_STREAM;
        protocol = IPPROTO_TCP;
    }
    else
    {
        // SOCK_DGRAM - creating a datagram-socket
        // IPPROTO_UDP - use UDP protocol
        socketType = SOCK_DGRAM;
        protocol = IPPROTO_UDP;
    }
    
    //5. create socket
    
    // Starting Winsock for Windows
    #ifndef __linux__
    WSADATA w;
    if(int result = WSAStartup(MAKEWORD(2,2), &w) != 0)
    {
        cerr << "Failed to start Winsock 2! Error #" << result << endl;
        return -1;
    }
    #endif

    // AF_INET - creating an IPv4 based socket
    int Socket = socket(AF_INET, socketType, protocol);
    if(Socket == -1)
    {
        cerr << "Error on creating socket: " << errno << endl;
        return -1;
    }
    else
    {
        cout << "Socket created for " << (protocol == IPPROTO_TCP ? "TCP" : "UDP") << endl;
    }
    
    //6. connect
    
    //local address
    sockaddr* local = createAddress(AF_INET, "", localPort);
    if(bind(Socket, local, sizeof(*local)) == -1)
    {
        cerr << "Error binding the socket: " << errno << endl;
        return -1;
    }
    else
    {
        cout << "Local port bound: " << localPort << endl;
    }
    
    //remote address
    sockaddr* remote = createAddress(AF_INET, ipString, destPort);
    if(connect(Socket,remote,sizeof(*remote)) == -1)
    {
        cerr << "Error connection the socket:" << errno << endl;
        return -1;
    }
    else
    {
        cout << "Connection established to " << ipString << ":" << destPort << endl;
    }
    
    return Socket;
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
    int socket = configureNetwork();
    if(socket < 0)
    {
        cerr << "Error in network configuration: " << errno << endl;
        return 1;
    }
    
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


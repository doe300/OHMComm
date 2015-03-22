/* 
 * File:   socket.cpp
 * Author: daniel
 *
 * Created on March 22, 2015, 12:07 PM
 * 
 * Source: https://www.c-plusplus.net/forum/169861-full
 */

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
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

//calls send() until the complete buffer is sent
void sendAll(int socket, const char* const buf, const int size)
{
    int bytesSent = 0; // number of already sent bytes
    do
    {
        bytesSent += send(socket, buf + bytesSent, size - bytesSent, 0);
    } while(bytesSent < size);
}

// Reads the input line for line
void getLine(int socket, std::stringstream& line)
{
    for(char c; recv(socket, &c, 1, 0) > 0; line << c)
    {
        if(c == '\n')
        {
            return;
        }
    }
    //this will only be called in case of an error (recv returning with -1)
    throw std::runtime_error("End of stream");
}

//retrieves the content of the remote address using HTTP protocol
void getHTTP(int socket, std::string host, std::string path)
{
    const std::string request = "GET "+path+" HTTP/1.1\r\nHost: "+host+"\r\n\r\n";
    
    sendAll(socket, request.c_str(), request.length());
    
    while(true)
    {
        stringstream line;
        try
        {
            getLine(socket, line);
        }
        catch(exception& e)
        {
            break;
        }
        cout << line.str() << endl;
    }
}

//retrieves the content of the remote address using RTP protocol
void getRTP(int socket, std::string path)
{
    //FIXME implement!!
}

/*
 * 
 */
int main(int argc, char** argv)
{
    using namespace std;
    
    ////
    // User input
    ////
    
    //user input for remote IP and port
    string ip;
    string portTemp;
    int port;
    cout << "destination IP: ";
    cin >> ip;
    cout << "destination port: ";
    cin >> portTemp;
    //convert port to number
    port = convertToInt(portTemp);
    
    //user input for connection type
    string type;
    int protocol;
    int socketType;
    cout << "connection type (TCP or UDP): ";
    cin >> type;
    
    if(type == "TCP")
    {
        socketType = SOCK_STREAM;
        protocol = IPPROTO_TCP;
    }
    else
    {
        socketType = SOCK_DGRAM;
        protocol = IPPROTO_UDP;
    }
    
    //user input for request
    string prot;
    string path;
    cout << "Protocol (HTTP or RTP): ";
    cin >> prot;
    cout << "path on server: ";
    cin >> path;
    
    ////
    // Creating socket
    ////
    
    // Starting Winsock for Windows
    #ifndef __linux__
    WSADATA w;
    if(int result = WSAStartup(MAKEWORD(2,2), &w) != 0)
    {
        cout << "Failed to start Winsock 2! Error #" << result << endl;
        return 1;
    }
    #endif

    // AF_INET - creating an IPv4 based socket
    // SOCK_STREAM - creating a stream-socket
    // IPPROTO_TCP - use TCP/IP protocol
    int Socket = socket(AF_INET, socketType, protocol);
    if(Socket == -1)
    {
        cout << "Error on creating socket!" << endl;
        return 1;
    }
    
    cout << "Socket created for " << type << endl;
    
    ////
    // Connecting
    ////
    
    //remote address
    sockaddr_in remote;
    //we want to connect to an IPv4 address
    remote.sin_family = AF_INET;
    //the remote IP address - inet_addr() is a convenience-method to convert from aaa.bbb.ccc.ddd (octal representation) to a 4-byte unsigned long
    remote.sin_addr.s_addr = inet_addr(ip.c_str());
    //network byte order is big-endian, so we must convert the port-number
    remote.sin_port = htons(port);
    
    //we must cast sockaddr_in to a sockaddr struct
    int result = connect(Socket,reinterpret_cast<sockaddr*>(&remote),sizeof(remote));
    
    if(result == -1)
    {
        cout << "Error connection the socket" << endl;
        return 1;
    }
    
    cout << "Connection established!" << endl;
    
    ////
    // Sending and receiving
    ////
    
    if(prot == "RTP")
    {
        getRTP(Socket,path);
    }
    else
    {
        getHTTP(Socket, ip, path);
    }
    
    ////
    // Closing socket
    ////
    
    #ifdef __linux__
    shutdown(Socket, SHUT_RDWR);
    #else
    closesocket(Socket);
    #endif

    cout << "Connection closed!" << endl;
    return 0;
}
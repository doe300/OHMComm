/* 
 * File:   Utility.cpp
 * Author: daniel
 * 
 * Created on December 5, 2015, 4:14 PM
 */

#include <iostream>

#include "Utility.h"
#include "sip/STUNClient.h"

#ifdef _WIN32
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#define STDIN_FILENO _fileno(stdin)
#else
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif

const unsigned int MAX_NAME_SIZE{ 255 };

std::string Utility::getDomainName()
{
    char hostName[MAX_NAME_SIZE];
    int status = gethostname(hostName, MAX_NAME_SIZE);
    if (status == 0)
    {
        return std::string(hostName);
    }
    return std::string("unknown");
}

std::string Utility::getUserName()
{
#ifdef _WIN32
    char userName[MAX_NAME_SIZE];
    DWORD nameSize = sizeof(userName);
    int status = GetUserName((LPTSTR)userName, &nameSize);
    if (status == 0)
    {
        return std::string(userName);
    }
#else
    uid_t userID = getuid();
    struct passwd *userInfo;
    userInfo = getpwuid(userID);
    if (userInfo != nullptr)
    {
        return std::string(userInfo->pw_name);
    }
#endif
	return std::string("unknown");
}

std::string Utility::getLocalIPAddress(const AddressType addressType)
{
    switch(addressType)
    {
        case AddressType::ADDRESS_LOOPBACK:
            return "127.0.0.1";
        case AddressType::ADDRESS_LOCAL_NETWORK:
            return getExternalLocalIPAddress();
        case AddressType::ADDRESS_INTERNET:
            return getExternalNetworkIPAddress();
    }
    throw std::invalid_argument("No AddressType constant");
}

Utility::AddressType Utility::getNetworkType(const std::string& remoteAddress)
{
    if(remoteAddress.compare("127.0.0.1") == 0 || remoteAddress.compare("::1") == 0 
       || remoteAddress.compare("0:0:0:0:0:0:0:1") == 0 || remoteAddress.compare("localhost") == 0)
    {
        return AddressType::ADDRESS_LOOPBACK;
    }
    //Check for private network address
    if(Utility::isLocalNetwork(remoteAddress))
    {
        return AddressType::ADDRESS_LOCAL_NETWORK;
    }
    //all other addresses are external
    return AddressType::ADDRESS_INTERNET;
}

std::string Utility::getAddressForHostName(const std::string& hostName)
{
    std::string ipAddress;
    addrinfo* info;
    if(getaddrinfo(hostName.c_str(), nullptr, nullptr, &info) != 0)
    {
        //handle error
        return "";
    }
    char buffer[64] = {0};
    addrinfo* cur = info;
    while(cur != nullptr)
    {
        if(info->ai_family == AF_INET && info->ai_addr != nullptr)
        {
            inet_ntop(info->ai_family, &(((sockaddr_in*)info->ai_addr)->sin_addr), buffer, 64);
            break;
        }
        else if(info->ai_family == AF_INET6 && info->ai_addr != nullptr)
        {
            inet_ntop(info->ai_family, &(((sockaddr_in6*)info->ai_addr)->sin6_addr), buffer, 64);
            break;
        }
        cur = cur->ai_next;
    }
    ipAddress = buffer;
    freeaddrinfo(info);
    return ipAddress;
}


std::string Utility::trim(const std::string& in)
{
    //https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    auto wsfront=std::find_if_not(in.begin(),in.end(), ::isspace);
    return std::string(wsfront,std::find_if_not(in.rbegin(),std::string::const_reverse_iterator(wsfront), ::isspace).base());
}

bool Utility::equalsIgnoreCase(const std::string& s1, const std::string s2)
{
    if(s1.size() != s2.size())
    {
        return false;
    }
#ifdef _WIN32
    return lstrcmpi((const wchar_t*)s1.data(), (const wchar_t*)s2.data()) == 0;
#else
    return strcasecmp(s1.data(), s2.data()) == 0;
#endif
}

std::string Utility::replaceAll(std::string str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string Utility::joinStrings(const std::vector<std::string>& vector, const std::string& delimiter)
{
    std::vector<std::string>::const_iterator it = vector.begin();
    std::string result(*it);
    ++it;
    for(;it != vector.end();++it)
    {
        result.append(delimiter);
        result.append(*it);
    }
    return result;
}

/* Converts a hex character to its integer value */
inline char from_hex(char ch)
{
    return ::isdigit(ch) ? ch - '0' : ::tolower(ch) - 'a' + 10;
}

std::string Utility::decodeURI(const std::string& uri)
{
    //Taken from: http://www.geekhideout.com/urlcode.shtml
    std::string result;
    const char* pstr = uri.c_str();
    while (*pstr != '\0')
    {
        if (*pstr == '%')
        {
            if (pstr[1] && pstr[2])
            {
                result += from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                pstr += 2;
            }
        }
        else if (*pstr == '+')
        {
            result += ' ';
        }
        else
        {
            result += *pstr;
        }
        pstr++;
    }
    return result;
}

std::string Utility::toHexString(unsigned int number)
{
    //a 32 bit number has at most 8 hexadecimal digits
    char tmp[8] = {'\0'};
    sprintf(tmp, "%X", number);
    return std::string(tmp);
}

int Utility::waitForUserInput(const int waitInMS)
{
    fd_set readFDs;
    FD_ZERO(&readFDs);
    FD_SET(STDIN_FILENO, &readFDs);
    timeval waitFor{waitInMS/1000, (waitInMS%1000)*1000};
    timeval* waitForPtr = waitInMS == -1 ? nullptr : &waitFor;
    const int result = select(1, &readFDs, nullptr, nullptr, waitForPtr);
    if(result)
    {
        return std::cin.get();
    }
    return -1;
}


std::string Utility::getExternalLocalIPAddress()
{
    //find external IP of local device
    std::string address("");
#ifdef _WIN32
    //See: http://tangentsoft.net/wskfaq/examples/ipaddr.html
    const hostent* phe = gethostbyname(getDomainName().c_str());
    char buffer[65] = {0};
    for (int i = 0; phe->h_addr_list[i] != nullptr; ++i)
    {
        inet_ntop(phe->h_addrtype, phe->h_addr_list[i], buffer, 64);
        break;
    }
    address = buffer;
#else
    //See: http://linux.die.net/man/3/getifaddrs
    ifaddrs* start = nullptr;
    ifaddrs* current = nullptr;
    
    if(getifaddrs(&start) == -1)
    {
        //error
        return "";
    }
    for(current = start; current != nullptr; current = current->ifa_next)
    {
        //skip loopback
        if((current->ifa_flags & IFF_LOOPBACK) == IFF_LOOPBACK)
        {
            continue;
        }
        //skip disabled
        if((current->ifa_flags & (IFF_UP|IFF_RUNNING)) == 0)
        {
            continue;
        }
        //skip nullptr on socket-address
        if(current->ifa_addr == nullptr)
        {
            continue;
        }
        //skip non IP
        if(current->ifa_addr->sa_family != AF_INET && current->ifa_addr->sa_family != AF_INET6)
        {
            continue;
        }
        //otherwise select first
        break;
    }
    if(current != nullptr)
    {
        char buffer[65] = {0};
        const sockaddr* socketAddress = current->ifa_addr;
        getnameinfo(socketAddress, (socketAddress->sa_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
                    buffer, 64, nullptr, 0, NI_NUMERICHOST );
        address = buffer;
    }
    //free allocated structures
    freeifaddrs(start);
#endif
    return address;
}

std::string Utility::getExternalNetworkIPAddress()
{
    //use STUN to get our external IP/port
    STUNClient stun;
    auto result = stun.retrieveSIPInfo();
    if(std::get<0>(result))
    {
        return std::get<1>(result);
    }
    return "";
    
    //old code, uses one single server and no standard protocol
//    //1. open TCP connection
//    TCPWrapper network(55555, "91.198.22.70", 80);
//    //2. send something - anything
//    const std::string message("anything\r\n\r\n");
//    network.sendData(message.c_str(), message.size());
//    //3. read response
//    char buffer[1024] = {0};
//    if(network.receiveData(buffer, 1023) == TCPWrapper::RECEIVE_TIMEOUT)
//    {
//        //we timed out - don't delay the application too much
//        return "";
//    }
//    std::string response = buffer;
//    //4. extract IP address
//    std::string::size_type index = response.find("<body>");
//    if(index != std::string::npos)
//    {
//        index = response.find(':', index) + 2;
//        return response.substr(index, response.find('<', index) - index);
//    }
//    return "";
}

bool Utility::isLocalNetwork(const std::string& ipAddress)
{
    //See: https://en.wikipedia.org/wiki/Private_network
    if(ipAddress.find('.') != std::string::npos)
    {
        //we have IPv4
        in_addr address;
        inet_pton(AF_INET, ipAddress.c_str(), &address);
        const uint32_t asNumber = ntohl(address.s_addr);
        //first block: 10.0.0.0 - 10.255.255.255 - check values 167772160 to 184549375
        if(asNumber >= 167772160 && asNumber <= 184549375)
        {
            return true;
        }
        //second block: 172.16.0.0 - 172.31.255.255 - check values 2886729728 to 2887778303
        if(asNumber >= 2886729728 && asNumber <= 2887778303)
        {
            return true;
        }
        //third block: 192.168.0.0 - 192.168.255.255 - check values 3232235520 to 3232301055
        if(asNumber >= 3232235520 && asNumber <= 3232301055)
        {
            return true;
        }
        return false;
    }
    else if(ipAddress.find(':') != std::string::npos)
    {
        //we have IPv6
        in6_addr address;
        inet_pton(AF_INET6, ipAddress.c_str(), &address);
        //local block: fd00::/8
        const uint8_t* firstByte = (const uint8_t*) &address;
        return *firstByte == 0xFD;
    }
    throw std::invalid_argument("No IPv4 or IPv6 address supplied");
}

/* 
 * File:   Utility.cpp
 * Author: daniel
 * 
 * Created on December 5, 2015, 4:14 PM
 */

#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <random>

#include "Utility.h"
#include "sip/STUNClient.h"
#include "network/NetworkGrammars.h"

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

using namespace ohmcomm;

static const unsigned int MAX_NAME_SIZE{255};

std::string Utility::getDomainName()
{
    char hostName[MAX_NAME_SIZE];
    int status = gethostname(hostName, MAX_NAME_SIZE);
    if (status == 0) {
        return std::string(hostName);
    }
    return std::string("unknown");
}

std::string Utility::getUserName()
{
#ifdef _WIN32
    char userName[MAX_NAME_SIZE];
    DWORD nameSize = sizeof (userName);
    int status = GetUserName((LPTSTR) userName, &nameSize);
    if (status == 0) {
        return std::string(userName);
    }
#else
    uid_t userID = getuid();
    struct passwd *userInfo;
    userInfo = getpwuid(userID);
    if (userInfo != nullptr) {
        return std::string(userInfo->pw_name);
    }
#endif
    return std::string("unknown");
}

std::string Utility::getLocalIPAddress(const AddressType addressType)
{
    switch (addressType) {
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
    if (remoteAddress.compare("127.0.0.1") == 0 || remoteAddress.compare("::1") == 0
        || remoteAddress.compare("0:0:0:0:0:0:0:1") == 0 || remoteAddress.compare("localhost") == 0) {
        return AddressType::ADDRESS_LOOPBACK;
    }
    //Check for private network address
    if (Utility::isLocalNetwork(remoteAddress)) {
        return AddressType::ADDRESS_LOCAL_NETWORK;
    }
    //all other addresses are external
    return AddressType::ADDRESS_INTERNET;
}

std::string Utility::getAddressForHostName(const std::string& hostName)
{
    if (ohmcomm::network::NetworkGrammars::isIPv4Address(hostName) || ohmcomm::network::NetworkGrammars::isIPv6Address(hostName))
        return hostName;

    std::string ipAddress;
    addrinfo* info;
    if (getaddrinfo(hostName.c_str(), nullptr, nullptr, &info) != 0) {
        //handle error
        return "";
    }
    char buffer[64] = {0};
    addrinfo* cur = info;
    while (cur != nullptr) {
        if (cur->ai_family == AF_INET && cur->ai_addr != nullptr) {
            inet_ntop(cur->ai_family, &(((sockaddr_in*) cur->ai_addr)->sin_addr), buffer, 64);
            break;
        }
        else if (cur->ai_family == AF_INET6 && cur->ai_addr != nullptr) {
            inet_ntop(cur->ai_family, &(((sockaddr_in6*) cur->ai_addr)->sin6_addr), buffer, 64);
            break;
        }
        cur = cur->ai_next;
    }
    ipAddress = buffer;
    freeaddrinfo(info);
    return ipAddress;
}

std::pair<std::string, unsigned short> Utility::getSocketAddress(const void* socketAddress, const unsigned int addressLength, const bool isIPv6)
{
    if (socketAddress == nullptr)
        throw std::invalid_argument("Socket address can't be nullptr!");
    char buffer[64] = {0};
    unsigned short port;
    if (isIPv6) {
        if (addressLength < sizeof (sockaddr_in6))
            throw std::invalid_argument("Socket address buffer too small!");
        const sockaddr_in6* ipv6Address = (sockaddr_in6*) socketAddress;
        inet_ntop(AF_INET6, (void*) &(ipv6Address->sin6_addr), buffer, 64);
        port = ntohs(ipv6Address->sin6_port);
    }
    else {
        if (addressLength < sizeof (sockaddr_in))
            throw std::invalid_argument("Socket address buffer too small!");
        const sockaddr_in* ipv4Address = (sockaddr_in*) socketAddress;
        inet_ntop(AF_INET, (void*) &(ipv4Address->sin_addr), buffer, 64);
        port = ntohs(ipv4Address->sin_port);
    }
    return std::make_pair(std::string(buffer), port);
}

std::string Utility::trim(const std::string& in)
{
    //https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    auto wsfront = std::find_if_not(in.begin(), in.end(), ::isspace);
    return std::string(wsfront, std::find_if_not(in.rbegin(), std::string::const_reverse_iterator(wsfront), ::isspace).base());
}

bool Utility::equalsIgnoreCase(const std::string& s1, const std::string s2)
{
    if (s1.size() != s2.size()) {
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
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string Utility::joinStrings(const std::vector<std::string>& vector, const std::string& delimiter)
{
    if (vector.empty()) {
        return "";
    }
    std::vector<std::string>::const_iterator it = vector.begin();
    std::string result(*it);
    ++it;
    for (; it != vector.end(); ++it) {
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
    while (*pstr != '\0') {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                result += from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                pstr += 2;
            }
        }
        else if (*pstr == '+') {
            result += ' ';
        }
        else {
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
    timeval waitFor{waitInMS / 1000, (waitInMS % 1000)*1000};
    timeval* waitForPtr = waitInMS == -1 ? nullptr : &waitFor;
    const int result = select(1, &readFDs, nullptr, nullptr, waitForPtr);
    if (result) {
        return std::cin.get();
    }
    return -1;
}

std::vector<std::string> Utility::splitString(const std::string& input, const char delimiter)
{
    std::vector<std::string> result;
    std::stringstream in(input);
    std::string token;
    while (std::getline(in, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

std::string Utility::generateRandomUUID()
{
    //[...]form 8-4-4-4-12 for a total of 36 characters
    char strUuid[37] = {};
    //adapted from https://stackoverflow.com/questions/2174768/generating-random-uuids-in-linux

    sprintf(strUuid, "%hx%hx-%hx-%hx-%hx-%hx%hx%hx",
            randomNumber(), randomNumber(), // Generates a 32-bit Hex number
            randomNumber(), // Generates a 16-bit Hex number
            ((randomNumber() & 0x0fff) | 0x4000), // Generates a 16-bit Hex number of the form 4xxx (4 indicates the UUID version)
            randomNumber() % 0x3fff + 0x8000, // Generates a 16-bit Hex number in the range [0x8000, 0xbfff]
            randomNumber(), randomNumber(), randomNumber()); // Generates a 48-bit Hex number

    return std::string(strUuid);
}

double Utility::prettifyPercentage(const double percentage)
{
    int tmp = percentage * 10000;
    return tmp / 100.0;
}

std::string Utility::prettifyByteSize(const double byteSize)
{
    if (byteSize < 0) {
        return "?";
    }
    std::string unit;
    double dimensionValue;
    if (byteSize > 1024 * 1024 * 1024) {
        unit = " GB";
        dimensionValue = byteSize / (1024.0 * 1024.0 * 1024.0);
    }
    else if (byteSize > 1024 * 1024) {
        unit = " MB";
        dimensionValue = byteSize / (1024.0 * 1024.0);
    }
    else if (byteSize > 1024) {
        unit = " KB";
        dimensionValue = byteSize / 1024.0;
    }
    else {
        unit = " B";
        dimensionValue = byteSize;
    }
    char buf[8];
    int numChars = std::sprintf(buf, "%.2f", dimensionValue);
    return std::string(buf, numChars) + unit;
}

//License for encodeBase64 and decodeBase64
/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

 */
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string Utility::encodeBase64(const std::string& plainString)
{
    //see: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    unsigned int in_len = plainString.size();
    const unsigned char* bytes_to_encode = reinterpret_cast<const unsigned char*> (plainString.data());

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;
}

std::string Utility::decodeBase64(const std::string& base64String)
{
    //see: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
    int in_len = base64String.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (base64String[in_] != '=') && is_base64(base64String[in_])) {
        char_array_4[i++] = base64String[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

unsigned int Utility::randomNumber()
{
    static std::mt19937 engine(std::chrono::system_clock::now().time_since_epoch().count());
    return engine();
}

std::string Utility::getExternalLocalIPAddress()
{
    //find external IP of local device
    std::string address("");
#ifdef _WIN32
    //See: http://tangentsoft.net/wskfaq/examples/ipaddr.html
    const hostent* phe = gethostbyname(getDomainName().c_str());
    char buffer[65] = {0};
    for (int i = 0; phe->h_addr_list[i] != nullptr; ++i) {
        inet_ntop(phe->h_addrtype, phe->h_addr_list[i], buffer, 64);
        break;
    }
    address = buffer;
#else
    //See: http://linux.die.net/man/3/getifaddrs
    ifaddrs* start = nullptr;
    ifaddrs* current = nullptr;

    if (getifaddrs(&start) == -1) {
        //error
        return "";
    }
    for (current = start; current != nullptr; current = current->ifa_next) {
        //skip loopback
        if ((current->ifa_flags & IFF_LOOPBACK) == IFF_LOOPBACK) {
            continue;
        }
        //skip disabled
        if ((current->ifa_flags & (IFF_UP | IFF_RUNNING)) == 0) {
            continue;
        }
        //skip nullptr on socket-address
        if (current->ifa_addr == nullptr) {
            continue;
        }
        //skip non IP
        if (current->ifa_addr->sa_family != AF_INET && current->ifa_addr->sa_family != AF_INET6) {
            continue;
        }
        //otherwise select first
        break;
    }
    if (current != nullptr) {
        char buffer[65] = {0};
        const sockaddr* socketAddress = current->ifa_addr;
        getnameinfo(socketAddress, (socketAddress->sa_family == AF_INET) ? sizeof (sockaddr_in) : sizeof (sockaddr_in6),
                    buffer, 64, nullptr, 0, NI_NUMERICHOST);
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
    sip::STUNClient stun;
    auto result = stun.retrieveSIPInfo();
    if (std::get<0>(result)) {
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
    if (ipAddress.find('.') != std::string::npos) {
        //we have IPv4
        in_addr address;
        inet_pton(AF_INET, ipAddress.c_str(), &address);
        const uint32_t asNumber = ntohl(address.s_addr);
        //first block: 10.0.0.0 - 10.255.255.255 - check values 167772160 to 184549375
        if (asNumber >= 167772160 && asNumber <= 184549375) {
            return true;
        }
        //second block: 172.16.0.0 - 172.31.255.255 - check values 2886729728 to 2887778303
        if (asNumber >= 2886729728 && asNumber <= 2887778303) {
            return true;
        }
        //third block: 192.168.0.0 - 192.168.255.255 - check values 3232235520 to 3232301055
        if (asNumber >= 3232235520 && asNumber <= 3232301055) {
            return true;
        }
        return false;
    }
    else if (ipAddress.find(':') != std::string::npos) {
        //we have IPv6
        in6_addr address;
        inet_pton(AF_INET6, ipAddress.c_str(), &address);
        //local block: fd00::/8
        const uint8_t* firstByte = (const uint8_t*) &address;
        return *firstByte == 0xFD;
    }
    throw std::invalid_argument("No IPv4 or IPv6 address supplied");
}

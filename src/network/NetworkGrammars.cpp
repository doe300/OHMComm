/* 
 * File:   NetworkGrammars.cpp
 * Author: daniel
 * 
 * Created on February 13, 2016, 10:30 AM
 */

#include <regex>
#include <stdlib.h>

#include "network/NetworkGrammars.h"

using namespace ohmcomm::network;

//due to some problem with clang++, this can't be constexpr (or declared in the header)
const int NetworkGrammars::INVALID_PORT{-1};

const auto flags = std::regex_constants::icase|std::regex_constants::optimize|std::regex_constants::ECMAScript;

std::tuple<std::string, int> NetworkGrammars::toHostAndPort(const std::string& hostAndPort, const int defaultPort)
{
    //there are several possibilities:
    //there is no ':', so we only have a host
    if(hostAndPort.find(':') == std::string::npos)
    {
        if(isValidHost(hostAndPort))
        {
            return std::make_tuple(hostAndPort, defaultPort);
        }
    }
    else    //there is a ':', so we either have a port or an IPv6 address
    {
        std::string::size_type lastIndex = hostAndPort.find_last_of(':');
        if(hostAndPort.find_first_of(':') == lastIndex) //there is only one ':'
        {
            const std::string hostPart = hostAndPort.substr(0, lastIndex);
            if(isValidHost(hostPart))
            {
                return std::make_tuple(hostPart, toPort(hostAndPort.substr(lastIndex + 1)));
            }
        }
        else   //there is an IPv6 address (maybe with port)
        {
            const std::string ipv6Part = hostAndPort.substr(0, lastIndex);
            if(isIPv6Address(ipv6Part)) //if the string without the last part is a valid IPv6 address, the last part must be a port
            {
                return std::make_tuple(ipv6Part, toPort(hostAndPort.substr(lastIndex + 1)));
            }
            if(isIPv6Address(hostAndPort))
            {
                return std::make_tuple(hostAndPort, defaultPort);
            }
        }
    }
    return std::make_tuple("", INVALID_PORT);
}

bool NetworkGrammars::isValidDomainName(const std::string& hostName)
{
    if(hostName.size() > 64)
        //domains can't be more than 64 characters
        return false;
    //TLD must be optional to allow for "localhost" and any other host-name which is not a domain-name (e.g. for local network)
    static const std::regex hostNameRegex{"^(([[:alnum:]]([[:alnum:]]|\\-)*[[:alnum:]])\\.)*([[:alnum:]]([[:alnum:]]|\\-)*[[:alnum:]])(\\.[[:alpha:]]{2,3}){0,1}$", flags};
    return std::regex_match(hostName, hostNameRegex, std::regex_constants::match_default);
}

bool NetworkGrammars::isIPv4Address(const std::string& address)
{
    static const std::regex ipv4Regex{"^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})$", flags};
    std::smatch result;
    if(!std::regex_match(address, result, ipv4Regex, std::regex_constants::match_default))
    {
        return false;
    }
    uint16_t firstByte = atoi(result.str(1).data());
    uint16_t lastByte = atoi(result.str(4).data());
    return firstByte != 0 && firstByte <= 255 && atoi(result.str(2).data()) <= 255 && atoi(result.str(3).data()) <= 255 && lastByte != 0 && lastByte <= 255;
}

bool NetworkGrammars::isIPv6Address(const std::string& address)
{
    //source: https://stackoverflow.com/questions/53497/regular-expression-that-matches-valid-ipv6-addresses#17871737
    static const std::regex ipv6Regex{"^" 
        "("
        "([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|"        // 1:2:3:4:5:6:7:8
        "([0-9a-fA-F]{1,4}:){1,7}:|"                       // 1::                              1:2:3:4:5:6:7::
        "([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|"       // 1::8             1:2:3:4:5:6::8  1:2:3:4:5:6::8
        "([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|"// 1::7:8           1:2:3:4:5::7:8  1:2:3:4:5::8
        "([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|"// 1::6:7:8         1:2:3:4::6:7:8  1:2:3:4::8
        "([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|"// 1::5:6:7:8       1:2:3::5:6:7:8  1:2:3::8
        "([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|"// 1::4:5:6:7:8     1:2::4:5:6:7:8  1:2::8
        "[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|"     // 1::3:4:5:6:7:8   1::3:4:5:6:7:8  1::8  
        ":((:[0-9a-fA-F]{1,4}){1,7}|:)|"                   // ::2:3:4:5:6:7:8  ::2:3:4:5:6:7:8 ::8       ::     
        "fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|"   // fe80::7:8%eth0   fe80::7:8%1     (link-local IPv6 addresses with zone index)
        "::(ffff(:0{1,4}){0,1}:){0,1}"
        "((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
        "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|"        // ::255.255.255.255   ::ffff:255.255.255.255  ::ffff:0:255.255.255.255  (IPv4-mapped IPv6 addresses and IPv4-translated addresses)
        "([0-9a-fA-F]{1,4}:){1,4}:"
        "((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
        "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])"         // 2001:db8:3:4::192.0.2.33  64:ff9b::192.0.2.33 (IPv4-Embedded IPv6 Address)
        ")$", flags};
    return std::regex_match(address, ipv6Regex, std::regex_constants::match_default);
    //TODO or use simpler check?? https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c#335263
}

int NetworkGrammars::toPort(const std::string& port)
{
    //maximum of 65535 -> 5 characters (when all leading zeros are ignored)
    static const std::regex portRegex{"^0*[0-9]{1,5}$", flags};
    if(!std::regex_match(port, portRegex, std::regex_constants::match_default))
    {
        return INVALID_PORT;
    }
    int portNum = atoi(port.c_str());
    if(portNum < 0 || portNum > UINT16_MAX)
        return INVALID_PORT;
    return portNum;
}


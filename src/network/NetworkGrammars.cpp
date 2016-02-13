/* 
 * File:   NetworkGrammars.cpp
 * Author: daniel
 * 
 * Created on February 13, 2016, 10:30 AM
 */

#include <regex>

#include "network/NetworkGrammars.h"

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
    static const std::regex hostNameRegex{"^((\\w(\\w|\\-)*\\w)\\.)*(\\w(\\w|\\-)*\\w)(\\.){0,1}$", flags};
    return std::regex_match(hostName, hostNameRegex, std::regex_constants::match_default);
}

bool NetworkGrammars::isIPv4Address(const std::string& address)
{
    static const std::regex ipv4Regex{"^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$", flags};
    return std::regex_match(address, ipv4Regex, std::regex_constants::match_default);
}

bool NetworkGrammars::isIPv6Address(const std::string& address)
{
    static const std::regex ipv6Regex{"^(([0-9]|%x41\\-46|%x61\\-66){1,4}(\\:([0-9]|%x41\\-46|%x61\\-66){1,4})*|([0-9]|%x41\\-46|%x61\\-66){1,4}(\\:([0-9]|%x41\\-46|%x61\\-66){1,4})*\\:\\:(([0-9]|%x41\\-46|%x61\\-66){1,4}(\\:([0-9]|%x41\\-46|%x61\\-66){1,4})*){0,1}|\\:\\:(([0-9]|%x41\\-46|%x61\\-66){1,4}(\\:([0-9]|%x41\\-46|%x61\\-66){1,4})*){0,1})(\\:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}){0,1}$", flags};
    return std::regex_match(address, ipv6Regex, std::regex_constants::match_default);
}

int NetworkGrammars::toPort(const std::string& port)
{
    static const std::regex portRegex{"^[0-9]+$", flags};
    if(!std::regex_match(port, portRegex, std::regex_constants::match_default))
    {
        return INVALID_PORT;
    }
    int portNum = atoi(port.c_str());
    if(portNum < 0 || portNum > UINT16_MAX)
        return INVALID_PORT;
    return portNum;
}


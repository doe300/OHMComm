/* 
 * File:   SIPGrammar.cpp
 * Author: daniel
 * 
 * Created on February 12, 2016, 2:25 PM
 */

#include <regex>
#include <sstream>

#include "Utility.h"
#include "sip/SIPGrammar.h"
#include "network/NetworkGrammars.h"

const std::string SIPGrammar::PROTOCOL_SIP("sip");
const std::string SIPGrammar::PROTOCOL_SIPS("sips");
const auto flags = std::regex_constants::icase|std::regex_constants::optimize|std::regex_constants::ECMAScript;

SIPGrammar::SIPURI SIPGrammar::readSIPURI(const std::string& sipURI, const unsigned short defaultPort)
{
    static const std::regex sipURIRegex{"^(sips?)\\:(([^:@\\s]+)(\\:([^@\\s]*)){0,1}@){0,1}([^\\;\\?]+)(;[^\\?]+)*(\\?.+){0,1}$", flags};
    static const std::regex userRegex{"^(([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66)|(&|\\=|\\+|\\$|,|;|\\?|/))+$", flags};
    static const std::regex passwordRegex{"(([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66)|&|\\=|\\+|\\$|,)*", flags};
    static const std::regex parameterRegex{"(((\\[|\\]|/|\\:|&|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))+)(\\=(((\\[|\\]|/|\\:|&|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))+)){0,1}", flags};
    static const std::regex headerRegex{"((((\\[|\\]|/|\\?|\\:|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))+)\\=(((\\[|\\]|/|\\?|\\:|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))*))", flags};
    //SIP-uri           = "sip"["s"]":" [ userinfo ] hostport
    //with 
    // userinfo         = ( username ) [ ":" password ] "@"
    // hostport         =  host [ ":" port ]
    // host             =  hostname / IPv4address / IPv6reference
    // port             =  1*DIGIT
    // ...
    std::smatch result;
    if(!std::regex_match(sipURI, result, sipURIRegex, std::regex_constants::match_default))
    {
        return SIPURI{};
    }
    //user may be empty
    if(!result.str(3).empty() && !std::regex_match(result.str(3), userRegex, std::regex_constants::match_default))
    {
        return SIPURI{};
    }
    //password may be empty
    if(!result.str(5).empty() && !std::regex_match(result.str(5), passwordRegex, std::regex_constants::match_default))
    {
        return SIPURI{};
    }
    if(result.str(6).empty())
    {
        return SIPURI{};
    }
    const std::tuple<std::string, int> hostAndPort = NetworkGrammars::toHostAndPort(result.str(6), defaultPort);
    //host must not be empty, port must not be invalid
    if(std::get<0>(hostAndPort).empty() || std::get<1>(hostAndPort) == NetworkGrammars::INVALID_PORT)
    {
        return SIPURI{};
    }
    SIPURI uri{};
    uri.protocol = result.str(1);
    uri.user = result.str(3);
    uri.password = result.str(5);
    uri.host = std::get<0>(hostAndPort);
    uri.port = std::get<1>(hostAndPort);
    
    if(!result.str(7).empty())  //there are URI parameters
    {
        //skip initial ';'
        std::string parametersString = result.str(7).substr(1);
        std::smatch parameterResult;
        while(std::regex_search(parametersString, parameterResult, parameterRegex))
        {
            uri.parameters[parameterResult.str(1)] = parameterResult.str(9);
            parametersString = parameterResult.suffix().str();
        }
    }
    
    if(!result.str(8).empty())  //there are headers
    {
        //skip initial '?'
        std::string headersString = result.str(8).substr(1);
        std::smatch headersResult;
        while(std::regex_search(headersString, headersResult, headerRegex))
        {
            uri.headers[headersResult.str(2)] = headersResult.str(9);
            headersString = headersResult.suffix().str();
        }
    }
    
    return uri;
}

std::string SIPGrammar::toSIPURI(const SIPURI& sipURI)
{
    std::stringstream ss;
    ss << sipURI.protocol << ":";
    if(!sipURI.user.empty())
    {
        ss << sipURI.user;
        if(!sipURI.password.empty())
        {
            ss << ':' << sipURI.password;
        }
        ss << '@';
    }
    ss << sipURI.host;
    if(sipURI.port > 0 && sipURI.port < UINT16_MAX)
    {
        ss << ':' << sipURI.port;
    }
    for(const SIPURIParameter& param : sipURI.parameters.fields)
    {
        ss << ';' << param.key;
        if(!param.value.empty())
        {
            ss << '=' << param.value;
        }
    }
    if(!sipURI.headers.fields.empty())
    {
        bool first = true;
        for(const SIPURIHeader& header : sipURI.headers.fields)
        {
            ss << (first ? '?' : '&') << header.key << '=' << header.value;
            first = false;
        }
    }
    return ss.str();
}

std::tuple<std::string, SIPGrammar::SIPURI> SIPGrammar::readNamedAddress(const std::string& namedAddress, const unsigned short defaultPort)
{
    const std::string::size_type openIndex = namedAddress.find('<');
    std::string namePart;
    if(openIndex == std::string::npos || namedAddress.find('>') == std::string::npos)
    {
        //there is no name-part
        namePart = "";
        return std::make_tuple("", SIPURI{});
    }
    else
    {
        std::string namePart = Utility::trim(namedAddress.substr(0, openIndex));
        if(!namePart.empty() && namePart[0] == '"' && namePart[namePart.size()-1] =='"')
        {
            //remove surrounding '"'s
            namePart = namePart.substr(1, namePart.size()-2);
        }
    }
    const std::string::size_type length = openIndex == std::string::npos ? namedAddress.size() : (namedAddress.find_last_of('>') - openIndex - 1);
    return std::make_tuple(namePart, readSIPURI(namedAddress.substr(openIndex + 1, length), defaultPort));
}

std::string SIPGrammar::toNamedAddress(const SIPURI& sipURI, const std::string& name)
{
    std::stringstream ss;
    if(!name.empty())
    {
        if(name.find_first_of(" \r\n\t") != std::string::npos)
            //if string contains spaces, surround with '"'
            ss << '"' << name << '"' << ' ';
        else
            ss << name << ' ';
    }
    ss << '<' << toSIPURI(sipURI) << '>';
    return ss.str();
}

std::tuple<std::string, SIPGrammar::SIPURI> SIPGrammar::readViaAddress(const std::string& viaField, const unsigned short defaultPort)
{
    static const std::regex viaRegex{"^([^\\s]+)\\s+([^\\s]+)$", flags};
    std::smatch result;
    if(!std::regex_match(viaField, result, viaRegex))
    {
        return std::make_tuple("", SIPURI{});
    }
    
    const std::string protocol = result.str(1);
    //protocol must be "SIP/2.0/<transport>"
    if(protocol.find("SIP/2.0/") != 0)
    {
        return std::make_tuple("", SIPURI{});
    }
    const SIPURI sipURI = readSIPURI(std::string("sip:") + result.str(2), defaultPort);
    //TODO test host for connectivity??
    return std::make_tuple(protocol, sipURI);
}

std::string SIPGrammar::toViaAddress(const SIPURI& sipURI, const std::string& protocolVersion)
{
    return (protocolVersion + " ") + toSIPURI(sipURI).substr(4);
}


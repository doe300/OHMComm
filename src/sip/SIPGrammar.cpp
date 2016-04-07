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

using namespace ohmcomm::sip;

const std::string SIPGrammar::PROTOCOL_SIP("sip");
const std::string SIPGrammar::PROTOCOL_SIPS("sips");
const auto flags = std::regex_constants::icase|std::regex_constants::optimize|std::regex_constants::ECMAScript;
//TODO rewrite to throw invalid_arguments!!

SIPGrammar::SIPURI SIPGrammar::readSIPURI(const std::string& sipURI, const unsigned short defaultPort)
{
    static const std::regex sipURIRegex{"^(sips?)\\:(([^:@\\s]+)(\\:([^@\\s]*)){0,1}@){0,1}([^\\;\\?]+)(;[^\\?]+)*(\\?.+){0,1}$", flags};
    static const std::regex userRegex{"^(([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66)|(&|\\=|\\+|\\$|,|;|\\?|/))+$", flags};
    static const std::regex passwordRegex{"^(([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66)|&|\\=|\\+|\\$|,)*$", flags};
    static const std::regex parameterRegex{"^(((\\[|\\]|/|\\:|&|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))+)(\\=(((\\[|\\]|/|\\:|&|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))+)){0,1}$", flags};
    static const std::regex headerRegex{"^((((\\[|\\]|/|\\?|\\:|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))+)\\=(((\\[|\\]|/|\\?|\\:|\\+|\\$)|([A-Za-z0-9]|(\\-|_|\\.|\\!|~|\\*|'|\\(|\\)))|%([0-9]|%x41\\-46|%x61\\-66)([0-9]|%x41\\-46|%x61\\-66))*))$", flags};
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
    //host[:port] MUST NOT be empty
    if(result.str(6).empty())
    {
        return SIPURI{};
    }
    const std::tuple<std::string, int> hostAndPort = ohmcomm::network::NetworkGrammars::toHostAndPort(result.str(6), defaultPort);
    //host must not be empty, port must not be invalid
    if(std::get<0>(hostAndPort).empty() || std::get<1>(hostAndPort) == ohmcomm::network::NetworkGrammars::INVALID_PORT)
    {
        return SIPURI{};
    }
    SIPURI uri{};
    uri.protocol = result.str(1);
    uri.user = result.str(3);
    uri.password = result.str(5);
    uri.host = std::get<0>(hostAndPort);
    uri.port = std::get<1>(hostAndPort);
    
    //URI parameters may be empty
    if(!result.str(7).empty())  //there are URI parameters
    {
        //skip initial ';'
        const std::vector<std::string> parameters = Utility::splitString(result.str(7).substr(1), ';');
        for(const std::string& parameter : parameters)
        {
            std::smatch parameterResult;
            if(!std::regex_match(parameter, parameterResult, parameterRegex))
            {
                return SIPURI{};
            }
            uri.parameters[parameterResult.str(1)] = parameterResult.str(9);
        }
    }
    
    //headers may be empty
    if(!result.str(8).empty())  //there are headers
    {
        //skip initial '?'
        const std::vector<std::string> headerFields = Utility::splitString(result.str(8).substr(1), '&');
        for(const std::string& headerField : headerFields)
        {
            std::smatch headersResult;
            if(!std::regex_match(headerField, headersResult, headerRegex))
            {
                return SIPURI{};
            }
            uri.headers[headersResult.str(2)] = headersResult.str(9);
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

SIPGrammar::SIPAddress SIPGrammar::readNamedAddress(const std::string& namedAddress, const unsigned short defaultPort)
{
    //syntax for SIP token (RFC 3261, section 25.1)
    static const std::regex tokenRegex{"^([[:alnum:]]|\\-|\\.|\\!|\\%|\\*|\\_|\\+|\\`|\\'|\\~)*$", flags};
    const std::string::size_type openIndex = namedAddress.find('<');
    SIPAddress result;
    if(openIndex == std::string::npos && namedAddress.find('>') == std::string::npos)
    {
        //there is no name-part
        result.displayName = "";
    }
    else
    {
        std::string namePart = Utility::trim(namedAddress.substr(0, openIndex));
        if(!namePart.empty() && namePart[0] == '"' && namePart[namePart.size()-1] =='"')
        {
            //remove surrounding '"'s
            namePart = namePart.substr(1, namePart.size()-2);
        }
        //check name syntax - only if name was not surrounded by quotes
        else if(!std::regex_match(namePart, tokenRegex))
        {
            //name-part does not match the grammar
            return {"", {}, {}};
        }
        result.displayName = namePart;
        //after the '>' there can be additional parameters
        //(";" token "=" (token|host|quoted-string))*
        std::string::size_type semiColonIndex = namedAddress.find(';', namedAddress.find('>'));
        std::string::size_type equalIndex;
        while(semiColonIndex != std::string::npos)
        {
            equalIndex = namedAddress.find('=', semiColonIndex);
            if(equalIndex == std::string::npos)
            {
                //parameter has no value, invalid
                return {"", {}, {}};
            }
            const std::string parameterKey = namedAddress.substr(semiColonIndex + 1, equalIndex - (semiColonIndex + 1));
            if(!std::regex_match(parameterKey, tokenRegex))
            {
                //parameter-key is wrong
                return {"", {}, {}};
            }
            //XXX out of simplicity, we currently accept any non-empty value
            semiColonIndex = namedAddress.find(';', semiColonIndex + 1);
            const std::string parameterValue = namedAddress.substr(equalIndex + 1, semiColonIndex - (equalIndex + 1));
            if(parameterValue.empty())
            {
                //parameter-value can't be empty
                return {"", {}, {}};
            }
            result.parameters[parameterKey] = parameterValue;
        }
    }
    const std::string::size_type length = openIndex == std::string::npos ? namedAddress.size() : (namedAddress.find_last_of('>') - openIndex - 1);
    result.uri = readSIPURI(namedAddress.substr(openIndex + 1, length), defaultPort);
    return result;
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
    return std::make_tuple(protocol, sipURI);
}

std::string SIPGrammar::toViaAddress(const SIPURI& sipURI, const std::string& protocolVersion)
{
    return (protocolVersion + " ") + toSIPURI(sipURI).substr(4);
}

bool SIPGrammar::isValidCallID(const std::string& callID)
{
    //syntax is: word [ "@" host ]
    static const std::regex wordRegex{"^([[:alnum:]]|\\-|\\.|\\!|\\%|\\*|\\_|\\+|\\`|\\'|\\~|\\(|\\)|\\<|\\>|\\:|\\\\|\\\"|\\/|\\[|\\]|\\?|\\{|\\})+$", flags};
    if(callID.empty())
        return false;
    const std::string::size_type index = callID.find('@');
    if(index != std::string::npos)
    {
        //need to check both parts
        if(index != callID.find_last_of('@'))
        {
            //more than one '@'
            return false;
        }
        if(!std::regex_match(callID.substr(0, index), wordRegex))
        {
            //first part doesn't match
            return false;
        }
        return ohmcomm::network::NetworkGrammars::isValidHost(callID.substr(index+1));
    }
    return std::regex_match(callID,wordRegex);
}

bool SIPGrammar::isValidCSeq(const std::string& cSeq)
{
    //syntax: number " " method
    const std::string::size_type index = cSeq.find(' ');
    if(index == std::string::npos)
        return false;
    //check first part as number (unsigned 32 bit integer, smaller 2^31)
    long sequenceNumber = strtol(cSeq.data(), nullptr, 10);
    if(sequenceNumber <= 0 || sequenceNumber >= INT32_MAX)
    {
        return false;
    }
    //check for existence of second part
    return !Utility::trim(cSeq.substr(index)).empty();
}

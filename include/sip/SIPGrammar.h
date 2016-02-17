/* 
 * File:   SIPGrammar.h
 * Author: daniel
 *
 * Created on February 12, 2016, 2:25 PM
 */

#ifndef SIPGRAMMAR_H
#define	SIPGRAMMAR_H

#include <string>
#include <tuple>

#include "KeyValuePairs.h"

/*!
 * Utility class to check and convert parts of the SIP protocol
 */
class SIPGrammar
{
public:

    struct SIPURIParameter : public KeyValuePair<std::string>
    {

        SIPURIParameter() : KeyValuePair()
        {
        }

        SIPURIParameter(std::string key, std::string value) : KeyValuePair(key, value)
        {
        }
    };

    struct SIPURIHeader : public KeyValuePair<std::string>
    {

        SIPURIHeader() : KeyValuePair()
        {
        }

        SIPURIHeader(std::string key, std::string value) : KeyValuePair(key, value)
        {
        }
    };
    
    struct SIPAddressParameter : public KeyValuePair<std::string>
    {
        SIPAddressParameter() : KeyValuePair()
        {
        }

        SIPAddressParameter(std::string key, std::string value) : KeyValuePair(key, value)
        {
        }
    };

    static const std::string PROTOCOL_SIP;
    static const std::string PROTOCOL_SIPS;

    struct SIPURI
    {
        std::string protocol;
        std::string user;
        std::string password;
        std::string host;
        int port;
        KeyValuePairs<SIPURIParameter> parameters;
        KeyValuePairs<SIPURIHeader> headers;
    };
    
    struct SIPAddress
    {
        std::string displayName;
        SIPURI uri;
        KeyValuePairs<SIPAddressParameter> parameters;
    };

    static SIPURI readSIPURI(const std::string& sipURI, const unsigned short defaultPort);

    static std::string toSIPURI(const SIPURI& sipURI);

    static SIPAddress readNamedAddress(const std::string& namedAddress, const unsigned short defaultPort);
    
    static std::string toNamedAddress(const SIPURI& sipURI, const std::string& name = "");
    
    static std::tuple<std::string, SIPURI> readViaAddress(const std::string& viaField, const unsigned short defaultPort);
    
    static std::string toViaAddress(const SIPURI& sipURI, const std::string& protocolVersion);
    
    static bool isValidCallID(const std::string& callID);
};

#endif	/* SIPGRAMMAR_H */


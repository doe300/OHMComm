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

namespace ohmcomm
{
    namespace sip
    {

        /*!
         * Utility class to check and convert parts of the SIP protocol
         */
        class SIPGrammar
        {
        public:

            using SIPURIParameter = KeyValuePair<std::string>;
            using SIPURIHeader = KeyValuePair<std::string>;
            using SIPAddressParameter = KeyValuePair<std::string>;

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

            static bool isValidCSeq(const std::string& cSeq);
            
            static std::string generateCallID(const std::string& host);
        };
    }
}
#endif	/* SIPGRAMMAR_H */


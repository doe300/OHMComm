/* 
 * File:   SIPUserAgent.h
 * Author: daniel
 *
 * Created on February 24, 2016, 11:58 AM
 */

#ifndef SIPUSERAGENT_H
#define	SIPUSERAGENT_H

#include <string>

/*!
 * Information about a participant of a session relevant for the SIP protocol
 */
struct SIPUserAgent
{
    std::string userName;
    std::string hostName;
    std::string tag;
    std::string ipAddress;
    unsigned short port;

    /*!
     * \return the SIP-URI in the format sip:<userName>@<hostName|ipAddress>[:<port>]
     */
    const inline std::string getSIPURI() const
    {
        //sip:user@host[:port]
        return (std::string("sip:") + userName + "@") + (hostName.empty() ? ipAddress : hostName) + (port > 0 ? std::string(":") + std::to_string(port): std::string());
    }
};

#endif	/* SIPUSERAGENT_H */


/* 
 * File:   SIPUserAgent.h
 * Author: daniel
 *
 * Created on December 9, 2015, 6:23 PM
 */

#ifndef SIPUSERAGENT_H
#define	SIPUSERAGENT_H

struct SIPUserAgent
{
    std::string userName;
    std::string hostName;
    std::string tag;
    std::string ipAddress;
    unsigned short port;
    
    const std::string getSIPURI() const;
};

extern SIPUserAgent sipUserAgents[2];

#endif	/* SIPUSERAGENT_H */


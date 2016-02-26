/* 
 * File:   SIPUserAgent.h
 * Author: daniel
 *
 * Created on February 24, 2016, 11:58 AM
 */

#ifndef SIPUSERAGENT_H
#define	SIPUSERAGENT_H

#include <string>
#include <map>

/*!
 * Information about a participant of a session relevant for the SIP protocol
 */
struct SIPUserAgent
{
    //XXX make const
    std::string tag;
    std::string userName;
    std::string hostName;
    std::string ipAddress;
    int64_t associatedSSRC;
    unsigned short port;
    
    SIPUserAgent(const std::string& tag) : tag(tag), userName(), hostName(), ipAddress(), associatedSSRC(-1), port(0)
    {
        
    }

    /*!
     * \return the SIP-URI in the format sip:<userName>@<hostName|ipAddress>[:<port>]
     */
    const inline std::string getSIPURI() const
    {
        //sip:user@host[:port]
        return (std::string("sip:") + userName + "@") + (hostName.empty() ? ipAddress : hostName) + (port > 0 ? std::string(":") + std::to_string(port): std::string());
    }
};

/*!
 * Local database for all active user agents
 */
class UserAgentDatabase
{
public:
    SIPUserAgent thisUA;
    
    UserAgentDatabase(const std::string& tag) : thisUA(tag)
    {
        
    }
    
    SIPUserAgent& getRemoteUA(const std::string& tag = "")
    {
        if(remoteAgents.find(tag) == remoteAgents.end())
            remoteAgents.insert(std::pair<std::string, SIPUserAgent>(tag, SIPUserAgent(tag)));
        return remoteAgents.at(tag);
    }
    
    bool isInUserAgentDB(const std::string& tag) const
    {
        return remoteAgents.find(tag) != remoteAgents.end();
    }
private:
    std::map<std::string, SIPUserAgent> remoteAgents;
};

#endif	/* SIPUSERAGENT_H */


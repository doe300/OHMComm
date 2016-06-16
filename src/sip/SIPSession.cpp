/* 
 * File:   SIPSession.cpp
 * Author: doe300
 * 
 * Created on June 12, 2016, 12:44 PM
 */

#include "sip/SIPSession.h"
#include "sip/SIPGrammar.h"
#include "network/MulticastNetworkWrapper.h"
#include "Utility.h"
#include "Logger.h"

using namespace ohmcomm::sip;

SIPSession::SIPSession(const ohmcomm::NetworkConfiguration& sipConfig, const std::string& remoteUser) : 
    userAgents(std::to_string(rand())), network(new ohmcomm::network::MulticastNetworkWrapper(sipConfig)), state(SessionState::DISCONNECTED)
{
    userAgents.thisUA.userName = ohmcomm::Utility::getUserName();
    userAgents.thisUA.hostName = ohmcomm::Utility::getDomainName();
    //we need an initial value for the local IP-address for the ";received="-tag
    userAgents.thisUA.ipAddress = ohmcomm::Utility::getLocalIPAddress(ohmcomm::Utility::getNetworkType(sipConfig.remoteIPAddress));
    userAgents.thisUA.tag = std::to_string(rand());
    userAgents.thisUA.port = sipConfig.localPort;
    userAgents.thisUA.callID = SIPGrammar::generateCallID(ohmcomm::Utility::getDomainName());
    SIPUserAgent& initialRemoteUA = userAgents.getRemoteUA();
    initialRemoteUA.userName = remoteUser;
    initialRemoteUA.ipAddress = sipConfig.remoteIPAddress;
    initialRemoteUA.port = sipConfig.remotePort;
}

SIPSession::~SIPSession()
{
}

void SIPSession::onRemoteConnected(const unsigned int ssrc, const std::string& address, const unsigned short port)
{
    //TODO remove
    //associate SIP user-data with RTP-participant
    SIPUserAgent* agent = userAgents.findForAddress(address, port);
    if(agent != nullptr)
    {
        agent->associatedSSRC = ssrc;
    }
}

void SIPSession::onRemoteRemoved(const unsigned int ssrc)
{
    //TODO remove
    //remove SIP user-agent for given SSRC
    SIPUserAgent* agent = userAgents.findForSSRC(ssrc);
    if(agent != nullptr)
    {
        userAgents.removeRemoteUA(agent->tag);
        //remove from destinations
        ohmcomm::network::MulticastNetworkWrapper* multicast = dynamic_cast<ohmcomm::network::MulticastNetworkWrapper*>(network.get());
        if(multicast != nullptr)
        {
            multicast->removeDestination(agent->ipAddress, agent->port);
        }
        ohmcomm::info("SIP") << "Remote removed: " << agent->getSIPURI() << ohmcomm::endl;
        if(userAgents.getNumberOfRemotes() == 0)
        {
            //last remote left
            shutdownInternal();
        }
    }
}

void SIPSession::initializeHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA, const NetworkConfiguration& sipConfig)
{
    //mandatory header-fields:
    //To, From, CSeq, Call-ID, Max-Forwards, Via, Contact
    header.fields.clear();
    //"rport" is specified in RFC3581 and requests the response to be sent to the originating port
    //"branch"-tag unique for all requests, randomly generated, starting with "z9hG4bK", ACK has same as INVITE (for non-2xx ACK) -> 8.1.1.7 Via
    //"received"-tag has the IP of the receiving endpoint
    remoteUA.lastBranch = requestHeader != nullptr ? requestHeader->getBranchTag() : (std::string("z9hG4bK") + std::to_string(rand()));
    const std::string receivedTag = requestHeader != nullptr ? std::string(";received=") + userAgents.thisUA.ipAddress : "";
    header[SIP_HEADER_VIA] = ((SIP_VERSION + "/UDP ") + userAgents.thisUA.hostName + ":") + ((((std::to_string(sipConfig.localPort)
                             + ";rport") + receivedTag) + ";branch=") + remoteUA.lastBranch);
    //TODO for response, copy/retain VIA-fields of request (multiple!)
    header[SIP_HEADER_CONTACT] = SIPGrammar::toNamedAddress(toSIPURI(userAgents.thisUA, false), userAgents.thisUA.userName);
    //if we INVITE, the remote call-ID is empty and we use ours, otherwise use the remote's
    header[SIP_HEADER_CALL_ID] =  remoteUA.callID.empty() ? userAgents.thisUA.callID : remoteUA.callID;
    try
    {
        SIPRequestHeader& reqHeader = dynamic_cast<SIPRequestHeader&>(header);
        ++remoteUA.sequenceNumber;
        reqHeader[SIP_HEADER_MAX_FORWARDS] = std::to_string(SIP_MAX_FORWARDS);
        reqHeader[SIP_HEADER_CSEQ] = (std::to_string(remoteUA.sequenceNumber) + " ") + requestMethod;

        //"tag"-tag for user-identification
        //if we INVITE, remote-tag is empty
        //if we got invited, remote-tag is correctly the tag sent by remote
        reqHeader[SIP_HEADER_TO] = SIPGrammar::toNamedAddress(toSIPURI(remoteUA, true), remoteUA.userName);
        reqHeader[SIP_HEADER_FROM] = SIPGrammar::toNamedAddress(toSIPURI(userAgents.thisUA, true), userAgents.thisUA.userName);
    }
    
    catch(const std::bad_cast& e)
    {
        //do nothing
    }
    try
    {
        SIPResponseHeader& resHeader = dynamic_cast<SIPResponseHeader&>(header);
        //CSeq of response if the same as request
        resHeader[SIP_HEADER_CSEQ] = (*requestHeader)[SIP_HEADER_CSEQ];
        //From/To has the value from the request
        if((*requestHeader)[SIP_HEADER_TO].find("tag=") == std::string::npos)
        {
            //on initial response, add tag
            resHeader[SIP_HEADER_TO] = ((*requestHeader)[SIP_HEADER_TO] + ";tag=") + userAgents.thisUA.tag;
        }
        else
        {
            resHeader[SIP_HEADER_TO] = (*requestHeader)[SIP_HEADER_TO];
        }
        resHeader[SIP_HEADER_FROM] = (*requestHeader)[SIP_HEADER_FROM];
    }
    catch(const std::bad_cast& e)
    {
        //do nothing
    }
    header[SIP_HEADER_USER_AGENT] = std::string("OHMComm/") + OHMCOMM_VERSION;
}

SIPGrammar::SIPURI SIPSession::toSIPURI(const SIPUserAgent& sipUA, const bool withParameters)
{
    SIPGrammar::SIPURI uri{"sip", sipUA.userName, "", sipUA.hostName.empty() ? sipUA.ipAddress : sipUA.hostName, sipUA.port};
    if(withParameters && !sipUA.tag.empty())
    {
        uri.parameters["tag"] = sipUA.tag;
    }
    return uri;
}
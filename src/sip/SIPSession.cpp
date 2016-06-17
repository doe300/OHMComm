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
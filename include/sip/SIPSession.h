/* 
 * File:   SIPSession.h
 * Author: doe300
 *
 * Created on June 12, 2016, 12:44 PM
 */

#ifndef SIPSESSION_H
#define SIPSESSION_H

#include <memory>
#include <functional>

#include "configuration.h"
#include "rtp/ParticipantDatabase.h"
#include "sip/SIPPackageHandler.h"
#include "sip/SDPMessageHandler.h"
#include "sip/SIPUserAgent.h"
#include "network/NetworkWrapper.h"

namespace ohmcomm
{
    namespace sip
    {

        class SIPSession : public rtp::ParticipantListener
        {
        public:
            
            //Function-type used to add a new user to the conversation
            using AddUserFunction = std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration)>;
            
            SIPSession(const ohmcomm::NetworkConfiguration& sipConfig, const std::string& remoteUser, const AddUserFunction addUserFunction);
            virtual ~SIPSession();
            
            void onRemoteConnected(const unsigned int ssrc, const std::string& address, const unsigned short port) override;
            void onRemoteRemoved(const unsigned int ssrc) override;
            
        protected:
            enum class SessionState
            {
                /*!
                 * Unknown status, we haven't established contact yet
                 */
                DISCONNECTED,
                /*!
                 * This UAC is currently trying to REGISTER with an UAS
                 */
                REGISTERING,
                /*!
                 * This UAC is registered with an UAC
                 */
                REGISTERED,
                /*!
                 * We sent an INVITE and are waiting for a response
                 */
                INVITING,
                /*!
                 * Session is established, communication is up and running.
                 * Don't accept any further INVITEs, only BYE
                 */
                ESTABLISHED,
                /*!
                 * We had a session and shut it down. Or we failed to initialize a session at all
                 */
                SHUTDOWN,
            };
            UserAgentDatabase userAgents;
            std::unique_ptr<ohmcomm::network::NetworkWrapper> network;
            SessionState state;
            
            virtual void shutdownInternal() = 0;
            
            /*!
             * Sets must-have header-fields
             */
            void initializeHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA, const NetworkConfiguration& sipConfig);
            
            int selectBestMedia(const std::vector<MediaDescription>& availableMedias) const;
            
            void startCommunication(const MediaDescription& descr, const NetworkConfiguration& rtpConfig, const NetworkConfiguration rtcpConfig);
            
        private:
            const AddUserFunction addUserFunction;
            static SIPGrammar::SIPURI toSIPURI(const SIPUserAgent& sipUA, const bool withParameters);
        };
    }
}
#endif /* SIPSESSION_H */


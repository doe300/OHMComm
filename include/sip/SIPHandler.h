/* 
 * File:   SIPHandler.h
 * Author: daniel
 *
 * Created on November 26, 2015, 12:37 PM
 */

#ifndef SIPHANDLER_H
#define	SIPHANDLER_H

#include <memory>
#include <thread>
#include <functional>
#include <exception>
#include <chrono>

#include "rtp/ParticipantDatabase.h"
#include "network/NetworkWrapper.h"
#include "SDPMessageHandler.h"
#include "SIPPackageHandler.h"
#include "SIPUserAgent.h"

namespace ohmcomm
{
    namespace sip
    {

        class SIPHandler : private ohmcomm::rtp::ParticipantListener
        {
        public:

            //A list of all allowed SIP-methods
            static const std::string SIP_ALLOW_METHODS;

            //A list of all accepted MIME-types
            static const std::string SIP_ACCEPT_TYPES;

            //The default port for SIP, as of RFC 3261
            static constexpr unsigned short SIP_DEFAULT_PORT{5060};

            SIPHandler(const NetworkConfiguration& sipConfig, const std::string& remoteUser, const std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration) > configFunction);

            ~SIPHandler();

            /*!
             * Shuts down the receive-thread
             */
            void shutdown();

            /*!
             * Starts the receive-thread
             */
            void startUp();

            /*!
             * \return whether the SIP-thread is up and running
             */
            bool isRunning() const;
            
            void onRemoteConnected(const unsigned int ssrc, const std::string& address, const unsigned short port) override;
            
            void onRemoteRemoved(const unsigned int ssrc) override;

            static std::string generateCallID(const std::string& host);

        private:
            static constexpr unsigned short SIP_BUFFER_SIZE{2048};

            enum class SessionState
            {
                /*!
                 * Unknown status, we haven't established contact yet
                 */
                UNKNOWN,
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
            NetworkConfiguration sipConfig;
            const std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration) > configFunction;
            std::function<void() > stopCallback = []()-> void
            {
            };
            std::vector<char> buffer;

            std::thread sipThread;
            bool threadRunning = false;
            SessionState state;

            /*!
             * Method called in the parallel thread, receiving SIP-packages and handling them
             */
            void runThread();

            void shutdownInternal();

            /*!
             * Sets must-have header-fields
             */
            void initializeHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA);

            void handleSIPRequest(const void* buffer, unsigned int packageLength, const ohmcomm::network::NetworkWrapper::Package& packageInfo);

            void handleSIPResponse(const void* buffer, unsigned int packageLength, const ohmcomm::network::NetworkWrapper::Package& packageInfo);

            void sendInviteRequest(SIPUserAgent& remoteUA);

            void sendCancelRequest(SIPUserAgent& remoteUA);

            void sendByeRequest(SIPUserAgent& remoteUA);

            void sendAckRequest(SIPUserAgent& remoteUA);

            void sendResponse(const unsigned int responseCode, const std::string reasonPhrase, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA);

            int selectBestMedia(const std::vector<MediaDescription>& availableMedias) const;

            void updateNetworkConfig(const SIPHeader* header, const ohmcomm::network::NetworkWrapper::Package* packageInfo, SIPUserAgent& remoteUA);

            void startCommunication(const MediaDescription& descr, const NetworkConfiguration& rtpConfig, const NetworkConfiguration rtcpConfig);

            friend class SIPConfiguration;
        };
    }
}
#endif	/* SIPHANDLER_H */


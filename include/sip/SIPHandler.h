/* 
 * File:   SIPHandler.h
 * Author: daniel
 *
 * Created on November 26, 2015, 12:37 PM
 */

#ifndef SIPHANDLER_H
#define	SIPHANDLER_H

#include <thread>
#include <exception>
#include <chrono>

#include "SIPSession.h"
#include "SIPRequest.h"

namespace ohmcomm
{
    namespace sip
    {

        class SIPHandler : private SIPSession
        {
        public:
            
            //The default port for SIP, as of RFC 3261
            static constexpr unsigned short SIP_DEFAULT_PORT{5060};
            
            //Function-type used to add a new user to the conversation
            using AddUserFunction = std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration)>;

            SIPHandler(const NetworkConfiguration& sipConfig, const std::string& remoteUser, const AddUserFunction addUserFunction);

            ~SIPHandler();
            
            /*!
             * Sets the user-data to register with at the remote server
             * 
             * NOTE: this method MUST be called before the SIP-thread is started to have any effect!
             * 
             * \since 1.0
             */
            void setRegisterWithServer(const std::string& registerUser = "", const std::string& registerPassword = "");

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
            
        protected:
            void shutdownInternal() override;
            
        private:
            static constexpr unsigned short SIP_BUFFER_SIZE{2048};
            std::string registerUser;
            std::string registerPassword;
            
            NetworkConfiguration sipConfig;
            const AddUserFunction addUserFunction;
            std::function<void() > stopCallback = []()-> void
            {
            };
            std::vector<char> buffer;

            std::thread sipThread;
            bool threadRunning = false;
            
            std::unique_ptr<SIPRequest> currentRequest;
            std::unique_ptr<Authentication> authentication;

            /*!
             * Method called in the parallel thread, receiving SIP-packages and handling them
             */
            void runThread();

            void handleSIPRequest(const void* buffer, unsigned int packageLength, const ohmcomm::network::NetworkWrapper::Package& packageInfo);
            
            void handleINVITERequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, std::string& requestBody, SIPUserAgent& remoteUA);
            
            void handleBYERequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, const std::string& requestBody, SIPUserAgent& remoteUA);
            
            void handleCANCELRequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, const std::string& requestBody, SIPUserAgent& remoteUA);
            
            void handleOPTIONSRequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, const std::string& requestBody, SIPUserAgent& remoteUA);
            
            void handleSIPResponse(const void* buffer, unsigned int packageLength, const ohmcomm::network::NetworkWrapper::Package& packageInfo);
            
            void sendInviteRequest(SIPUserAgent& remoteUA);

            void sendCancelRequest(SIPUserAgent& remoteUA);

            void sendByeRequest(SIPUserAgent& remoteUA);

            void sendAckRequest(SIPUserAgent& remoteUA);
            
            void sendRegisterRequest(SIPUserAgent& registerUA);
            
            void sendUnregisterRequest(SIPUserAgent& registerUA);
            
            void sendResponse(const unsigned int responseCode, const std::string reasonPhrase, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA);

            void updateNetworkConfig(const SIPHeader* header, const ohmcomm::network::NetworkWrapper::Package* packageInfo, SIPUserAgent& remoteUA);
            
            void startCommunication(const MediaDescription& descr, const NetworkConfiguration& rtpConfig, const NetworkConfiguration rtcpConfig);

            friend class SIPConfiguration;
        };
    }
}
#endif	/* SIPHANDLER_H */


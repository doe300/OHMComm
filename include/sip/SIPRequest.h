/* 
 * File:   SIPRequest.h
 * Author: doe300
 *
 * Created on June 12, 2016, 2:00 PM
 */

#ifndef SIPREQUEST_H
#define SIPREQUEST_H

#include "configuration.h"
#include "network/NetworkWrapper.h"
#include "SIPPackageHandler.h"
#include "SIPUserAgent.h"
#include "SIPGrammar.h"
#include "Authentication.h"
#include "SIPSession.h"

namespace ohmcomm
{
    namespace sip
    {
        
        //A list of all allowed SIP-methods
        const std::string SIP_ALLOW_METHODS = ohmcomm::Utility::joinStrings({SIP_REQUEST_INVITE, SIP_REQUEST_ACK, SIP_REQUEST_BYE, SIP_REQUEST_CANCEL, SIP_REQUEST_OPTIONS, SIP_REQUEST_INFO}, " ");

        //A list of all accepted MIME-types
        const std::string SIP_ACCEPT_TYPES = ohmcomm::Utility::joinStrings({MIME_SDP, MIME_MULTIPART_MIXED, MIME_MULTIPART_ALTERNATIVE}, ", ");

        //A list of all supported SIP extension headers
        const std::string SIP_SUPPORTED_FIELDS = ohmcomm::Utility::joinStrings({});

        //A list of all supported capabilities, as of RFC 3840 section 10
        //XXX sip.methods (one for each supported method)
        const std::string SIP_CAPABILITIES = ohmcomm::Utility::joinStrings({";sip.audio", "sip.duplex=full"}, ";");
        
        /*!
         * Sets must-have header-fields
         */
        void initializeSIPHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader, const SIPUserAgent& localUA, SIPUserAgent& remoteUA, const unsigned short localPort);
        SIPGrammar::SIPURI toSIPURI(const SIPUserAgent& sipUA, const bool withParameters);

        /*!
         * A SIP request this client sent away and waits for a response or another client sent to this program
         */
        class SIPRequest
        {
        public:
            SIPRequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network);
            virtual ~SIPRequest();
            
            /*!
             * Sends a request
             */
            virtual bool sendRequest(const std::string& requestBody) = 0;
            
            /*!
             * Handles the reception of a response
             */
            virtual bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) = 0;

            /*!
             * Handles the reception of a request
             */
            virtual bool handleRequest(const std::string& requestBody) = 0;
            
            /*!
             * \return whether this request has completed (successfully or failed)
             */
            virtual bool isCompleted() const = 0;
            
            bool isMatchingResponse(const SIPResponseHeader& header) const;
            
        protected:
            const SIPUserAgent& thisUA;
            SIPRequestHeader requestHeader;
            SIPUserAgent& remoteUA;
            const unsigned short localPort;
            ohmcomm::network::NetworkWrapper* network;
            
            void sendSimpleResponse(const unsigned int responseCode, const std::string reasonPhrase);
        };
        
        class REGISTERRequest : public SIPRequest
        {
        public:
            
            //default timeout for registrations, in seconds
            static constexpr unsigned short SIP_REGISTRATION_TIMEOUT{3600};
            
            REGISTERRequest(const SIPUserAgent& thisUA, SIPUserAgent& registerUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network, const std::string& userName, const std::string& password, const unsigned short expiresInSeconds = SIP_REGISTRATION_TIMEOUT);
            virtual ~REGISTERRequest();

            bool sendRequest(const std::string& requestBody) override;
            bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) override;
            bool handleRequest(const std::string& requestBody) override;
            bool isCompleted() const override;
            
            std::unique_ptr<Authentication> getAuthentication();

        private:
            const unsigned short expiresInSeconds;
            const std::string userName;
            const std::string password;
            std::unique_ptr<Authentication> authentication;
            
            SIPRequestHeader createRequest() const;
        };
        
        class OPTIONSRequest : public SIPRequest
        {
        public:
            OPTIONSRequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network);
            virtual ~OPTIONSRequest();
            
            bool sendRequest(const std::string& requestBody) override;
            bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) override;
            bool handleRequest(const std::string& requestBody) override;
            bool isCompleted() const override;

        };
        
        class INFORequest : public SIPRequest
        {
        public:
            INFORequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network);
            virtual ~INFORequest();

            bool sendRequest(const std::string& requestBody) override;
            bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) override;
            bool handleRequest(const std::string& requestBody) override;
            bool isCompleted() const override;
        };
        
        class BYERequest : public SIPRequest
        {
        public:
            BYERequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network);
            virtual ~BYERequest();

            bool sendRequest(const std::string& requestBody) override;
            bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) override;
            bool handleRequest(const std::string& requestBody) override;
            bool isCompleted() const override;
        };
        
        class CANCELRequest : public SIPRequest
        {
        public:
            CANCELRequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network);
            virtual ~CANCELRequest();
            
            bool sendRequest(const std::string& requestBody) override;
            bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) override;
            bool handleRequest(const std::string& requestBody) override;
            bool isCompleted() const override;
        };
        
        class INVITERequest : public SIPRequest
        {
        public:
            
            using ConnectCallback = std::function<void(const MediaDescription& descr, const NetworkConfiguration& rtpConfig, const NetworkConfiguration rtcpConfig)>;
            
            INVITERequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network, const SIPSession::SessionState state, const ConnectCallback callback);
            virtual ~INVITERequest();
            
            bool sendRequest(const std::string& requestBody) override;
            bool handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA) override;
            bool handleRequest(const std::string& requestBody) override;
            bool isCompleted() const override;
            
            SIPSession::SessionState getState() const;
        private:
            const ConnectCallback connectCallback;
            SIPSession::SessionState state;
            
            void sendAckRequest();
        };
    }
}

#endif /* SIPREQUEST_H */


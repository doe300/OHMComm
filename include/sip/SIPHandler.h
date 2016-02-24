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

#include "network/NetworkWrapper.h"
#include "SDPMessageHandler.h"
#include "SIPPackageHandler.h"

class SIPHandler
{
public:
    
    //A list of all allowed SIP-methods
    static const std::string SIP_ALLOW_METHODS;
    
    //A list of all accepted MIME-types
    static const std::string SIP_ACCEPT_TYPES;
    
    //The default port for SIP, as of RFC 3261
    static constexpr unsigned short SIP_DEFAULT_PORT{5060};
    
    SIPHandler(const NetworkConfiguration& sipConfig, const std::string& remoteUser, const std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration)> configFunction);

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
    std::unique_ptr<NetworkWrapper> network;
    NetworkConfiguration sipConfig;
    const std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration)> configFunction;
    std::function<void()> stopCallback = []()-> void{};
    std::string callID;
    uint32_t sequenceNumber;
    std::vector<char> buffer;
    std::string lastBranch;
    
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
    void initializeHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader);
    
    void handleSIPRequest(const void* buffer, unsigned int packageLength, const NetworkWrapper::Package& packageInfo);
    
    void handleSIPResponse(const void* buffer, unsigned int packageLength, const NetworkWrapper::Package& packageInfo);
    
    void sendInviteRequest();
    
    void sendCancelRequest();
    
    void sendByeRequest();
    
    void sendAckRequest();
    
    void sendResponse(const unsigned int responseCode, const std::string reasonPhrase, const SIPRequestHeader* requestHeader);
    
    int selectBestMedia(const std::vector<MediaDescription>& availableMedias) const;
    
    void updateNetworkConfig(const SIPHeader* header = nullptr, const NetworkWrapper::Package* packageInfo = nullptr);
    
    void startCommunication(const MediaDescription& descr, const NetworkConfiguration& rtpConfig, const NetworkConfiguration rtcpConfig);
    
    friend class SIPConfiguration;
};

#endif	/* SIPHANDLER_H */


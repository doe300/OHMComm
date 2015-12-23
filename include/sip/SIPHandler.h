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

#include "UDPWrapper.h"
#include "rtp/ParticipantDatabase.h"
#include "SDPMessageHandler.h"
#include "SIPPackageHandler.h"

//TODO some reasonable value
#define SIP_BUFFER_SIZE 4096

//A list of all allowed SIP-methods
const std::string SIP_ALLOW_METHODS("INVITE ACK BYE");

//A list of all accepted MIME-types
const std::string SIP_ACCEPT_TYPES(MIME_SDP);

const unsigned short SIP_DEFAULT_PORT =5060;

class SIPHandler
{
public:
    
    SIPHandler(const NetworkConfiguration& sipConfig, const std::string& remoteUser, const std::function<void(const MediaDescription&)> configFunction);
    
    SIPHandler(const NetworkConfiguration& sipConfig, const std::string& localUser, const std::string& localHostName, const std::string& remoteUser, const std::string& callID);

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
    std::unique_ptr<NetworkWrapper> network;
    const NetworkConfiguration sipConfig;
    const std::function<void(const MediaDescription&)> configFunction;
    std::function<void()> stopCallback = []()-> void{};
    SDPMessageHandler sdpHandler;
    std::string callID;
    uint32_t sequenceNumber;
    std::vector<char> buffer;
    
    std::thread sipThread;
    bool threadRunning = false;
    bool sessionEstablished = false;

    /*!
     * Method called in the parallel thread, receiving SIP-packages and handling them
     */
    void runThread();
    
    void shutdownInternal();
    
    /*!
     * Sets must-have header-fields
     */
    void initializeHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader);
    
    void handleSIPRequest(const void* buffer, unsigned int packageLength);
    
    void handleSIPResponse(const void* buffer, unsigned int packageLength);
    
    void sendInviteRequest();
    
    void sendByeRequest();
    
    void sendAckRequest();
    
    void sendResponse(const unsigned int responseCode, const std::string reasonPhrase, const SIPRequestHeader* requestHeader);
    
    const int selectBestMedia(const std::vector<MediaDescription>& availableMedias) const;
    
    void updateNetworkConfig();
    
    void startCommunication(const MediaDescription& descr);
    
    friend class SIPConfiguration;
};

#endif	/* SIPHANDLER_H */


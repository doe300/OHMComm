/* 
 * File:   SIPHandler.cpp
 * Author: daniel
 * 
 * Created on November 26, 2015, 12:37 PM
 */

#include <algorithm>

#include "Logger.h"
#include "sip/SIPHandler.h"
#include "network/MulticastNetworkWrapper.h"

using namespace ohmcomm::sip;

SIPHandler::SIPHandler(const ohmcomm::NetworkConfiguration& sipConfig, const std::string& remoteUser, const AddUserFunction addUserFunction, const std::string& registerUser, const std::string& registerPassword) : 
    SIPSession(sipConfig, remoteUser), registerUser(registerUser), registerPassword(registerPassword), sipConfig(sipConfig), addUserFunction(addUserFunction), buffer(SIP_BUFFER_SIZE)
{
    updateNetworkConfig(nullptr, nullptr, userAgents.getRemoteUA());
}

SIPHandler::~SIPHandler()
{
    // Wait until thread has really stopped
    sipThread.join();
}

void SIPHandler::startUp()
{
    threadRunning = true;
    sipThread = std::thread(&SIPHandler::runThread, this);
}

bool SIPHandler::isRunning() const
{
    return threadRunning;
}

void SIPHandler::shutdown()
{
    if(state == SessionState::INVITING)
    {
        //if we send an unanswered INVITE, CANCEL it
        sendCancelRequest(userAgents.getRemoteUA());
    }
    else if(state == SessionState::ESTABLISHED)
    {
        // Send a SIP BYE-packet, to tell the other side that communication has been stopped
        sendByeRequest(userAgents.getRemoteUA());
    }
    if(authentication && authentication->isAuthenticated())
    {
        //unregister from UAC
        sendUnregisterRequest(userAgents.getRemoteUA());
        //FIXME doesn't work, network is closed while making unregister-call
    }
    stopCallback();
    shutdownInternal();
}

void SIPHandler::shutdownInternal()
{
    // notify the thread to stop
    threadRunning = false;
    state = SessionState::SHUTDOWN;
    // close the socket
    network->closeNetwork();
}

void SIPHandler::runThread()
{
    ohmcomm::rtp::ParticipantDatabase::registerListener(*this);
    ohmcomm::info("SIP") << "SIP-Handler started ..." << ohmcomm::endl;
    state = SessionState::DISCONNECTED;
    
    //register on UAS, if specified
    if(!registerUser.empty())
    {
        if(userAgents.getRemoteUA().hostName.empty())
        {
            userAgents.getRemoteUA().hostName = userAgents.getRemoteUA().ipAddress;
        }
        userAgents.getRemoteUA().userName = registerUser;
        userAgents.thisUA.userName = registerUser;
        userAgents.thisUA.hostName = userAgents.getRemoteUA().hostName;
        sendRegisterRequest(userAgents.getRemoteUA());
    }
    else
    {
        //how to determine if initiating or receiving side??
        //doesn't really matter, if we are the first to start, the other side won't receive our INVITE, otherwise we INVITE
        sendInviteRequest(userAgents.getRemoteUA());
    }

    while (threadRunning)
    {
        //wait for package and store it in the SIPPackageHandler
        const ohmcomm::network::NetworkWrapper::Package result = network->receiveData(buffer.data(), buffer.size());
        if (threadRunning == false || result.isInvalidSocket())
        {
            //socket was already closed
            shutdownInternal();
        }
        else if (result.hasTimedOut())
        {
            //just continue to next loop iteration, checking if thread should continue running
        }
        else if (SIPPackageHandler::isRequestPackage(buffer.data(), result.getReceivedSize()))
        {
            handleSIPRequest(buffer.data(), result.getReceivedSize(), result);
        }
        else if (SIPPackageHandler::isResponsePackage(buffer.data(), result.getReceivedSize()))
        {
            handleSIPResponse(buffer.data(), result.getReceivedSize(), result);
        }
        if(authentication && authentication->getExpirationTime() - std::chrono::system_clock::now() < std::chrono::seconds(15))
        {
            //refresh authentication 15 seconds before it expires (should be enough time)
            if(!currentRequest || dynamic_cast<REGISTERRequest*>(currentRequest.get()) == nullptr)
            {
                //only if we are not currently trying to authenticate
                //otherwise, the refreshing could be initiated several times
                ohmcomm::info("SIP") << "Refreshing registration ..." << ohmcomm::endl;
                sendRegisterRequest(userAgents.getRemoteUA());
            }
        }
    }
    state = SessionState::SHUTDOWN;
    ohmcomm::info("SIP") << "SIP-Handler shut down!" << ohmcomm::endl;
    ohmcomm::rtp::ParticipantDatabase::unregisterListener(*this);
}

void SIPHandler::sendInviteRequest(SIPUserAgent& remoteUA)
{
    NetworkConfiguration rtpConfig = sipConfig;
    rtpConfig.localPort = DEFAULT_NETWORK_PORT;
    rtpConfig.remotePort = DEFAULT_NETWORK_PORT;
    const std::string messageBody = SDPMessageHandler::createSessionDescription(userAgents.thisUA.userName, rtpConfig);
    
    const INVITERequest::ConnectCallback callback = std::bind(&SIPHandler::startCommunication, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    currentRequest.reset(new INVITERequest(userAgents.thisUA, {}, remoteUA, sipConfig.localPort, network.get(), state, callback));
    currentRequest->sendRequest(messageBody);
    
    state = SessionState::INVITING;
}

void SIPHandler::sendCancelRequest(SIPUserAgent& remoteUA)
{
    currentRequest.reset(new CANCELRequest(userAgents.thisUA, {}, remoteUA, sipConfig.localPort, network.get()));
    currentRequest->sendRequest("");
    
    //TODO not every cancel results in closed connection, does it?
    state = SessionState::SHUTDOWN;
}

void SIPHandler::sendResponse(const unsigned int responseCode, const std::string reasonPhrase, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA)
{
    SIPResponseHeader header(responseCode, reasonPhrase);
    initializeSIPHeaderFields("", header, requestHeader, userAgents.thisUA, remoteUA, sipConfig.localPort);
    const std::string message = SIPPackageHandler::createResponsePackage(header, "");
    network->sendData(message.data(), message.size());
}

void SIPHandler::sendByeRequest(SIPUserAgent& remoteUA)
{
    currentRequest.reset(new BYERequest(userAgents.thisUA, {}, remoteUA, sipConfig.localPort, network.get()));
    currentRequest->sendRequest("");
    
    state = SessionState::SHUTDOWN;
}

void SIPHandler::sendAckRequest(SIPUserAgent& remoteUA)
{
    SIPRequestHeader header(SIP_REQUEST_ACK, remoteUA.getSIPURI());
    initializeSIPHeaderFields(SIP_REQUEST_ACK, header, nullptr, userAgents.thisUA, remoteUA, sipConfig.localPort);

    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    network->sendData(message.data(), message.size());
}

void SIPHandler::sendRegisterRequest(SIPUserAgent& registerUA)
{
    currentRequest.reset(new REGISTERRequest(userAgents.thisUA, registerUA, sipConfig.localPort, network.get(), registerUser, registerPassword));
    currentRequest->sendRequest("");
}

void SIPHandler::sendUnregisterRequest(SIPUserAgent& registerUA)
{
    //an Expiration header of zero specifies unregistration
    currentRequest.reset(new REGISTERRequest(userAgents.thisUA, registerUA, sipConfig.localPort, network.get(), registerUser, registerPassword, 0));
    currentRequest->sendRequest("");
}

void SIPHandler::handleSIPRequest(const void* buffer, unsigned int packageLength, const ohmcomm::network::NetworkWrapper::Package& packageInfo)
{
    SIPRequestHeader requestHeader;
    std::string requestBody;
    SIPUserAgent& remoteUA = userAgents.getRemoteUA(/*XXXrequestHeader.getRemoteTag()*/);
    try
    {
        requestBody = SIPPackageHandler::readRequestPackage(buffer, packageLength, requestHeader);
        ohmcomm::info("SIP") << "Request received: " << requestHeader.requestCommand << " " << requestHeader.requestURI << ohmcomm::endl;
        userAgents.getRemoteUA().tag = requestHeader.getRemoteTag();
        SIPPackageHandler::checkSIPHeader(&requestHeader);
        if(!requestHeader.hasKey(SIP_HEADER_MAX_FORWARDS) || atoi(requestHeader[SIP_HEADER_MAX_FORWARDS].data()) <= 0)
        {
            //too many hops
            sendResponse(SIP_RESPONSE_TOO_MANY_HOPS_CODE, SIP_RESPONSE_TOO_MANY_HOPS, &requestHeader, remoteUA);
            return;
        }
    }
    catch(const std::invalid_argument& error)
    {
        //fired on invalid IP-address or any error while parsing request
        //TODO error-response is sent to wrong (old) destination if the request came from another address
        ohmcomm::warn("SIP") << "Received bad request: " << error.what() << ohmcomm::endl;
        sendResponse(SIP_RESPONSE_BAD_REQUEST_CODE, SIP_RESPONSE_BAD_REQUEST, &requestHeader, remoteUA);
        return;
    }
    try
    {
        if (SIP_REQUEST_INVITE.compare(requestHeader.requestCommand) == 0)
        {
            handleINVITERequest(packageInfo, requestHeader, requestBody, remoteUA);
        }
        else if (SIP_REQUEST_BYE.compare(requestHeader.requestCommand) == 0)
        {
            handleBYERequest(packageInfo, requestHeader, requestBody, remoteUA);
        }
        else if(SIP_REQUEST_ACK.compare(requestHeader.requestCommand) == 0)
        {
            // remote has acknowledged our last message
            // we currently have no need to wait for confirmation
        }
        else if(SIP_REQUEST_CANCEL.compare(requestHeader.requestCommand) == 0)
        {
            handleCANCELRequest(packageInfo, requestHeader, requestBody, remoteUA);
        }
        else if(SIP_REQUEST_OPTIONS.compare(requestHeader.requestCommand) == 0)
        {
            handleOPTIONSRequest(packageInfo, requestHeader, requestBody, remoteUA);
        }
        else if(SIP_REQUEST_INFO.compare(requestHeader.requestCommand) == 0)
        {
            INFORequest info(userAgents.thisUA, requestHeader, remoteUA, sipConfig.localPort, network.get());
            info.handleRequest(requestBody);
        }
        else
        {
            ohmcomm::warn("SIP") << "Received not implemented request-method: " << requestHeader.requestCommand << " " << requestHeader.requestURI << ohmcomm::endl;
            //by default, send method not allowed
            sendResponse(SIP_RESPONSE_NOT_IMPLEMENTED_CODE, SIP_RESPONSE_NOT_IMPLEMENTED, &requestHeader, remoteUA);
            state = SessionState::DISCONNECTED;
        }
    }
    catch(const std::exception& error)
    {
        sendResponse(SIP_RESPONSE_SERVER_ERROR_CODE, SIP_RESPONSE_SERVER_ERROR, &requestHeader, remoteUA);
        ohmcomm::error("SIP") << "Server-error: " << error.what() << ohmcomm::endl;
        shutdown();
    }
}

void SIPHandler::handleINVITERequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, std::string& requestBody, SIPUserAgent& remoteUA)
{
    if(state != SessionState::ESTABLISHED)
    {
        updateNetworkConfig(&requestHeader, &packageInfo, remoteUA);
    }
    const INVITERequest::ConnectCallback callback = std::bind(&SIPHandler::startCommunication, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    currentRequest.reset(new INVITERequest(userAgents.thisUA, requestHeader, remoteUA, sipConfig.localPort, network.get(), state, callback));
    currentRequest->handleRequest(requestBody);
}

void SIPHandler::handleBYERequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, const std::string& requestBody, SIPUserAgent& remoteUA)
{
    ohmcomm::info("SIP") << "BYE received, shutting down ..." << ohmcomm::endl;
    //send ok to verify reception
    BYERequest bye(userAgents.thisUA, requestHeader, remoteUA, sipConfig.localPort, network.get());
    bye.handleRequest(requestBody);
    userAgents.removeRemoteUA(remoteUA.tag);
    //end communication, if established
    shutdownInternal();
    stopCallback();
}

void SIPHandler::handleCANCELRequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, const std::string& requestBody, SIPUserAgent& remoteUA)
{
    //remote has canceled their INVITE
    //if we have already established communication - do nothing
    if(state != SessionState::ESTABLISHED)
    {
        ohmcomm::info("SIP") << "CANCEL received, canceling session setup ..." << ohmcomm::endl;
        CANCELRequest cancel(userAgents.thisUA, requestHeader, remoteUA, sipConfig.localPort, network.get());
        //send ok to verify reception
        cancel.handleRequest(requestBody);
        userAgents.removeRemoteUA(remoteUA.tag);
        if(userAgents.getNumberOfRemotes() == 0)
        {
            //our initial call failed, shut down
            shutdownInternal();
            stopCallback();
        }
    }
}

void SIPHandler::handleOPTIONSRequest(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPRequestHeader& requestHeader, const std::string& requestBody, SIPUserAgent& remoteUA)
{
    //remote has requested OPTIONS about our capabilities
    //"The response code chosen MUST be the same that would have been chosen had the request been an INVITE.
    // That is, a 200 (OK) would be returned if the UAS is ready to accept a call, a 486 (Busy Here)
    // would be returned if the UAS is busy, etc."
    if(state != SessionState::ESTABLISHED)
    {
        updateNetworkConfig(&requestHeader, &packageInfo, remoteUA);
    }
    //if we get invited, the Call-ID is set by the remote UAS
    remoteUA.callID = requestHeader[SIP_HEADER_CALL_ID];

    OPTIONSRequest options(userAgents.thisUA, requestHeader, remoteUA, sipConfig.localPort, network.get());
    options.handleRequest(requestBody);
}

void SIPHandler::handleSIPResponse(const void* buffer, unsigned int packageLength, const ohmcomm::network::NetworkWrapper::Package& packageInfo)
{
    SIPResponseHeader responseHeader;
    std::string responseBody;
    try
    {
        responseBody = SIPPackageHandler::readResponsePackage(buffer, packageLength, responseHeader);
        ohmcomm::info("SIP") << "Response received: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << ohmcomm::endl;
        //TODO if we invite somebody, the ACK for the OK has no user-name set, because the info is updated later
        //workaround:
        if(userAgents.getRemoteUA().userName.empty())
            userAgents.getRemoteUA().userName = responseHeader.getAddress().uri.user;
        userAgents.getRemoteUA().tag = responseHeader.getRemoteTag();
        SIPPackageHandler::checkSIPHeader(&responseHeader);
    }
    catch(const std::invalid_argument& error)
    {
        //fired on invalid IP-address or any error while parsing request
        ohmcomm::warn("SIP") << "Received bad response: " << error.what() << ohmcomm::endl;
        return;
    }
    SIPUserAgent& remoteUA = userAgents.getRemoteUA(/*XXXrequestHeader.getRemoteTag()*/);
    try
    {
        if(currentRequest && currentRequest->isMatchingResponse(responseHeader) && currentRequest->handleResponse(packageInfo.address, responseHeader, responseBody, remoteUA))
        {
            //if previous request handles response, no need for extra handling
            if(currentRequest->isCompleted())
            {
                if(dynamic_cast<INVITERequest*>(currentRequest.get()) != nullptr)
                {
                    state = (dynamic_cast<INVITERequest*>(currentRequest.get()))->getState();
                    if(state == SIPSession::SessionState::ESTABLISHED)
                    {
                        //update remote SIP-URI
                        updateNetworkConfig(nullptr, nullptr, remoteUA);
                    }
                }
                else if(dynamic_cast<REGISTERRequest*>(currentRequest.get()) != nullptr)
                {
                    authentication = dynamic_cast<REGISTERRequest*>(currentRequest.get())->getAuthentication();
                    if(authentication && authentication->isAuthenticated())
                    {
                        ohmcomm::info("SIP") << "Authentication successful!" << ohmcomm::endl;
                        //TODO save authorization and unregister on shutdown (if not yet timed out)
                    }
                    else
                    {
                        ohmcomm::warn("SIP") << "Authentication failed!" << ohmcomm::endl;
                    }
                }
                currentRequest.reset();
            }
        }
        //TODO move default response handling to SIPRequest
        else if(responseHeader.statusCode >= 100 && responseHeader.statusCode < 200)
        {
            //for all provisional status-codes - do nothing
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_OK_CODE)
        {
            //no OK from a non-INVITE supported!
            ohmcomm::warn("SIP") << "Currently not supported: " << responseHeader.getRequestCommand() << ohmcomm::endl;
            ohmcomm::warn("SIP") << "Please file a bug report at: " << OHMCOMM_HOMEPAGE << ohmcomm::endl;
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_MULTIPLE_CHOICES_CODE || responseHeader.statusCode == SIP_RESPONSE_AMBIGUOUS_CODE)
        {
            //select first choice (Contact) and try again
            updateNetworkConfig(&responseHeader, &packageInfo, remoteUA);
            //TODO resend the current request
            sendInviteRequest(remoteUA);
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_MOVED_PERMANENTLY_CODE || responseHeader.statusCode == SIP_RESPONSE_MOVED_TEMPORARILY_CODE)
        {
            //change remote-address and retry
            updateNetworkConfig(&responseHeader, &packageInfo, remoteUA);
            //TODO resend the current request
            sendInviteRequest(remoteUA);
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_BAD_REQUEST_CODE)
        {
            ohmcomm::warn("SIP") << "We sent bad request: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << ohmcomm::endl;
            ohmcomm::warn("SIP") << "Please file a bug report at: " << OHMCOMM_HOMEPAGE << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_UNAUTHORIZED_CODE || responseHeader.statusCode == SIP_RESPONSE_PROXY_AUTHENTICATION_REQUIRED_CODE)
        {
            ohmcomm::warn("SIP") << "Server requires authorization!" << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_FORBIDDEN_CODE)
        {
            ohmcomm::warn("SIP") << "Access forbidden!" << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_NOT_FOUND_CODE)
        {
            ohmcomm::warn("SIP") << "Request-URI not found!" << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_METHOD_NOT_ALLOWED_CODE)
        {
            ohmcomm::warn("SIP") << "Server didn't allow our request-method!" << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_NOT_ACCEPTABLE_CODE || responseHeader.statusCode == SIP_RESPONSE_NOT_ACCEPTABLE_HERE_CODE)
        {
            ohmcomm::warn("SIP") << "Request could not be accepted: " << responseHeader.reasonPhrase << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_REQUEST_TIMEOUT_CODE || responseHeader.statusCode == SIP_RESPONSE_GONE_CODE)
        {
            ohmcomm::warn("SIP") << "Could not reach communication partner!" << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_REQUEST_ENTITY_TOO_LARGE_CODE || responseHeader.statusCode == SIP_RESPONSE_REQUEST_URI_TOO_LONG_CODE
                || responseHeader.statusCode == SIP_RESPONSE_UNSUPPORTED_MEDIA_TYPE_CODE || responseHeader.statusCode == SIP_RESPONSE_UNSUPPORTED_SCHEME_CODE)
        {
            ohmcomm::warn("SIP") << "Remote could not handle our request:" << responseHeader.statusCode << " " << responseHeader.reasonPhrase << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_EXTENSION_REQUIRED_CODE)
        {
            ohmcomm::warn("SIP") << "Remote requires an extension, we do not support!" << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_TEMPORARILY_UNAVAILABLE_CODE)
        {
            ohmcomm::warn("SIP") << "Communication partner temporarily unavailable: " << responseHeader.reasonPhrase << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_LOOP_DETECTED_CODE || responseHeader.statusCode == SIP_RESPONSE_TOO_MANY_HOPS_CODE)
        {
            ohmcomm::warn("SIP") << "Network-error: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_BUSY_CODE || responseHeader.statusCode == SIP_RESPONSE_BUSY_EVERYWHERE_CODE
                || responseHeader.statusCode == SIP_RESPONSE_DECLINE_CODE)
        {
            //remote is busy or call was declined
            ohmcomm::warn("SIP") << "Could not establish connection: " << responseHeader.reasonPhrase << ohmcomm::endl;
            sendAckRequest(remoteUA);
            state  = SessionState::SHUTDOWN;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_REQUEST_TERMINATED_CODE)
        {
            //we are working on an already terminated request
            shutdown();
        }
        else if(responseHeader.statusCode >= 500 && responseHeader.statusCode < 600)
        {
            //for all remote server errors - notify and shut down
            ohmcomm::warn("SIP") << "Remote server error: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << ohmcomm::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_DOES_NOT_EXIST_ANYWHERE_CODE)
        {
            ohmcomm::warn("SIP") << "Requested user does not exist!" << ohmcomm::endl;
            shutdown();
        }
        else
        {
            ohmcomm::warn("SIP") << "Received unsupported response: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << ohmcomm::endl;
            ohmcomm::warn("SIP") << "Please file a bug report at: " << OHMCOMM_HOMEPAGE << ohmcomm::endl;
        }
    }
    catch(const std::exception& error)
    {
        sendResponse(SIP_RESPONSE_SERVER_ERROR_CODE, SIP_RESPONSE_SERVER_ERROR, nullptr, remoteUA);
        ohmcomm::error("SIP") << "Server-error: " << error.what() << ohmcomm::endl;
        shutdown();
    }
}

void SIPHandler::updateNetworkConfig(const SIPHeader* header, const ohmcomm::network::NetworkWrapper::Package* packageInfo, SIPUserAgent& remoteUA)
{
    if(header != nullptr)
    {
        //set values for remote user/host and remote-address into SIP user agent database
        //this enables receiving INVITES from user agents other than the one, we sent the initial INVITE to
        const SIPGrammar::SIPAddress remoteAddress = header->getAddress();
        remoteUA.userName = remoteAddress.displayName;
        remoteUA.hostName = remoteAddress.uri.host;
        if(packageInfo != nullptr)
        {
            //use the actual address/port from the package received
            //NOTE: this is the easiest and fastest way to determine host/port, but may be inaccurate for some special cases
            //e.g. when remote uses different input/output ports or the package was meant to forward to another host
            const auto socketAddress = ohmcomm::Utility::getSocketAddress(&(packageInfo->address.ipv6), sizeof(packageInfo->address.ipv6), packageInfo->address.isIPv6);
            remoteUA.ipAddress = socketAddress.first;
            remoteUA.port = socketAddress.second;
        }
        else
        {
            remoteUA.ipAddress = Utility::getAddressForHostName(remoteAddress.uri.host);
            remoteUA.port = remoteAddress.uri.port == -1 ? SIP_DEFAULT_PORT : remoteAddress.uri.port;
        }
        if(remoteUA.ipAddress.empty())
        {
            ohmcomm::error("SIP") << "No address found for host: " << remoteAddress.uri.host << ohmcomm::endl;
            throw std::invalid_argument("Invalid IP address!");
        }
    }
    //check if configuration has changed
    if(sipConfig.remotePort != remoteUA.port || remoteUA.ipAddress.compare(sipConfig.remoteIPAddress) != 0)
    {
        ohmcomm::info("SIP") << "Reconnecting SIP ..." << ohmcomm::endl;
        //reset network-wrapper to send new packages to correct address
        sipConfig.remoteIPAddress = remoteUA.ipAddress;
        sipConfig.remotePort = remoteUA.port;
        network->closeNetwork();
        ohmcomm::info("SIP") << "Connecting to: " << sipConfig.remoteIPAddress << ':' << sipConfig.remotePort << ohmcomm::endl;
        //wait for socket to be closed
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        network.reset(new ohmcomm::network::UDPWrapper(sipConfig));
        
        //update all configuration-dependant values
        userAgents.thisUA.ipAddress = ohmcomm::Utility::getLocalIPAddress(ohmcomm::Utility::getNetworkType(sipConfig.remoteIPAddress));
    }
}

void SIPHandler::startCommunication(const MediaDescription& descr, const ohmcomm::NetworkConfiguration& rtpConfig, const ohmcomm::NetworkConfiguration rtcpConfig)
{
    state = SessionState::ESTABLISHED;
    ohmcomm::info("SIP") << "Using following SDP configuration: "
            << descr.protocol << " " << descr.payloadType << " " << descr.encoding << '/' << descr.sampleRate << '/' << descr.numChannels
            << ohmcomm::endl;
    if(rtcpConfig.remotePort != 0)
    {
        if(!rtcpConfig.remoteIPAddress.empty())
        {
            ohmcomm::info("SIP") << "Using custom remote RTCP address: " << rtcpConfig.remoteIPAddress << ":" << rtcpConfig.remotePort << ohmcomm::endl;
        }
        else
        {
            ohmcomm::info("SIP") << "Using custom remote RTCP port: " << rtcpConfig.remotePort << ohmcomm::endl;
        }
    }
    addUserFunction(descr, rtpConfig, rtcpConfig);
}
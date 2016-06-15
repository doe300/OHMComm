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

const std::string SIPHandler::SIP_ALLOW_METHODS = ohmcomm::Utility::joinStrings({SIP_REQUEST_INVITE, SIP_REQUEST_ACK, SIP_REQUEST_BYE, SIP_REQUEST_CANCEL, SIP_REQUEST_OPTIONS, SIP_REQUEST_INFO}, " ");
const std::string SIPHandler::SIP_ACCEPT_TYPES = ohmcomm::Utility::joinStrings({MIME_SDP, MIME_MULTIPART_MIXED, MIME_MULTIPART_ALTERNATIVE}, ", ");
const std::string SIPHandler::SIP_SUPPORTED_FIELDS = ohmcomm::Utility::joinStrings({});
//XXX sip.methods (one for each supported method)
const std::string SIPHandler::SIP_CAPABILITIES = ohmcomm::Utility::joinStrings({";sip.audio", "sip.duplex=full"}, ";");

SIPHandler::SIPHandler(const ohmcomm::NetworkConfiguration& sipConfig, const std::string& remoteUser, const AddUserFunction addUserFunction, const std::string& registerUser) : 
    SIPSession(sipConfig, remoteUser, addUserFunction), registerUser(registerUser), sipConfig(sipConfig), buffer(SIP_BUFFER_SIZE)
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
    
    currentRequest.reset(new INVITERequest(userAgents.thisUA, {}, remoteUA, sipConfig.localPort, network.get()));
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
    initializeHeaderFields("", header, requestHeader, remoteUA, sipConfig);
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
    initializeHeaderFields(SIP_REQUEST_ACK, header, nullptr, remoteUA, sipConfig);

    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    network->sendData(message.data(), message.size());
}

void SIPHandler::sendRegisterRequest(SIPUserAgent& registerUA)
{
    currentRequest.reset(new REGISTERRequest(userAgents.thisUA, {}, registerUA, sipConfig.localPort, network.get()));
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
    //if we get invited, the Call-ID is set by the remote UAS
    remoteUA.callID = requestHeader[SIP_HEADER_CALL_ID];
    //0. send ringing
    sendResponse(SIP_RESPONSE_RINGING_CODE, SIP_RESPONSE_RINGING, &requestHeader, remoteUA);
    //1. send dialog established
    sendResponse(SIP_RESPONSE_DIALOG_ESTABLISHED_CODE, SIP_RESPONSE_DIALOG_ESTABLISHED, &requestHeader, remoteUA);
    if (state == SessionState::ESTABLISHED)
    {
        //2.1 send busy here if communication is already running
        sendResponse(SIP_RESPONSE_BUSY_CODE, SIP_RESPONSE_BUSY, &requestHeader, remoteUA);
        ohmcomm::info("SIP") << "Received INVITE, but communication already active, answering: " << SIP_RESPONSE_BUSY << ohmcomm::endl;
    }
    else if (requestHeader[SIP_HEADER_CONTENT_TYPE].compare(MIME_SDP) == 0 || SIPPackageHandler::hasMultipartBody(requestHeader))
    {
        //adds support for multipart containing SDP (RFC 5621)
        if(SIPPackageHandler::hasMultipartBody(requestHeader))
        {
            //we only need the SDP part of the body
            requestBody = SIPPackageHandler::readMultipartBody(requestHeader, requestBody)[MIME_SDP];
            if(requestBody.empty())
            {
                ohmcomm::warn("SIP") << "Received multi-part INVITE without SDP body!" << ohmcomm::endl;
                shutdownInternal();
                return;
            }
        }
        SessionDescription sdp = SDPMessageHandler::readSessionDescription(requestBody);
        const std::vector<MediaDescription> availableMedias = SDPMessageHandler::readMediaDescriptions(sdp);
        //select best media
        int bestMediaIndex = selectBestMedia(availableMedias);
        if(bestMediaIndex < 0)
        {
            //no useful media found - send message (something about not-supported or similar)
            sendResponse(SIP_RESPONSE_NOT_ACCEPTABLE_CODE, SIP_RESPONSE_NOT_ACCEPTABLE, &requestHeader, remoteUA);
            state = SessionState::DISCONNECTED;
            shutdown();
            return;
        }
        //2.2 send ok to start communication (with selected media-format)
        SIPResponseHeader responseHeader(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
        initializeHeaderFields(SIP_REQUEST_INVITE, responseHeader, &requestHeader, remoteUA, sipConfig);
        responseHeader[SIP_HEADER_CONTENT_TYPE] = MIME_SDP;
        //RFC 3261: "A 2xx response to an INVITE SHOULD contain the Allow header field and the Supported header field, 
        // and MAY contain the Accept header field"
        responseHeader[SIP_HEADER_ALLOW] = SIP_ALLOW_METHODS;
        responseHeader[SIP_HEADER_ACCEPT] = SIP_ACCEPT_TYPES;
        responseHeader[SIP_HEADER_SUPPORTED] = SIP_SUPPORTED_FIELDS;
        NetworkConfiguration rtpConfig;
        rtpConfig.remoteIPAddress = sdp.getConnectionAddress();
        rtpConfig.localPort = DEFAULT_NETWORK_PORT;
        rtpConfig.remotePort = availableMedias[bestMediaIndex].port;
        const std::string messageBody = SDPMessageHandler::createSessionDescription(userAgents.thisUA.userName, rtpConfig, {availableMedias[bestMediaIndex]});
        const std::string message = SIPPackageHandler::createResponsePackage(responseHeader, messageBody);

        ohmcomm::info("SIP") << "Accepting INVITE from " << requestHeader[SIP_HEADER_CONTACT] << ohmcomm::endl;
        network->sendData(message.data(), message.size());

        //start communication
        startCommunication(availableMedias[bestMediaIndex], rtpConfig, SDPMessageHandler::readRTCPAttribute(sdp));
    }
    else
    {
        ohmcomm::warn("SIP") << "Received INVITE without SDP body!" << ohmcomm::endl;
        if(requestHeader.getContentLength() > 0)
        {
            ohmcomm::warn("SIP") << requestHeader.getContentLength() << " " << requestHeader[SIP_HEADER_CONTENT_TYPE] << ohmcomm::endl;
        }
        sendResponse(SIP_RESPONSE_NOT_ACCEPTABLE_CODE, SIP_RESPONSE_NOT_ACCEPTABLE, &requestHeader, remoteUA);
    }
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
        if(currentRequest && currentRequest->isMatchingResponse(responseHeader) && currentRequest->handleResponse(packageInfo, responseHeader, responseBody, remoteUA))
        {
            //if previous request handles response, no need for extra handling
            if(currentRequest->isCompleted())
            {
                currentRequest.reset();
            }
        }
        else if(responseHeader.statusCode >= 100 && responseHeader.statusCode < 200)
        {
            //for all provisional status-codes - do nothing
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_OK_CODE)
        {
            handleOKResponse(packageInfo, responseHeader, responseBody, remoteUA);
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_MULTIPLE_CHOICES_CODE || responseHeader.statusCode == SIP_RESPONSE_AMBIGUOUS_CODE)
        {
            //select first choice (Contact) and try again
            updateNetworkConfig(&responseHeader, &packageInfo, remoteUA);
            sendInviteRequest(remoteUA);
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_MOVED_PERMANENTLY_CODE || responseHeader.statusCode == SIP_RESPONSE_MOVED_TEMPORARILY_CODE)
        {
            //change remote-address and retry
            updateNetworkConfig(&responseHeader, &packageInfo, remoteUA);
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
            //TODO make info and try to authorize
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

void SIPHandler::handleOKResponse(const ohmcomm::network::NetworkWrapper::Package& packageInfo, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    //we have success - acknowledge reception
    sendAckRequest(remoteUA);
    //action depending on request
    const std::string requestCommand = responseHeader.getRequestCommand();
    if (SIP_REQUEST_INVITE.compare(requestCommand) == 0 && (responseHeader[SIP_HEADER_CONTENT_TYPE].compare(MIME_SDP) == 0 || SIPPackageHandler::hasMultipartBody(responseHeader)))
    {
        //adds support for multipart containing SDP (RFC 5621)
        if(SIPPackageHandler::hasMultipartBody(responseHeader))
        {
            //we only need the SDP part of the body
            responseBody = SIPPackageHandler::readMultipartBody(responseHeader, responseBody)[MIME_SDP];
            if(responseBody.empty())
            {
                ohmcomm::warn("SIP") << "Received multi-part OK without SDP body!" << ohmcomm::endl;
                shutdownInternal();
                return;
            }
        }
        NetworkConfiguration rtpConfig;

        rtpConfig.localPort = DEFAULT_NETWORK_PORT;
        //OK for INVITES contains SDP with selected media, as well as IP/session-data of remote
        const SessionDescription sdp = SDPMessageHandler::readSessionDescription(responseBody);
        rtpConfig.remoteIPAddress = sdp.getConnectionAddress();
        //extract MEDIA-data for selected media-type
        const std::vector<MediaDescription> selectedMedias = SDPMessageHandler::readMediaDescriptions(sdp);
        unsigned short mediaIndex = 0;
        //select best media
        if (selectedMedias.size() < 1) {
            ohmcomm::warn("SIP") << "Could not agree on audio-configuration, aborting!" << ohmcomm::endl;
            shutdownInternal();
        }
        //support for SDP offer/answer model (RFC 3264)
        //if medias > 1, select best media-type
        mediaIndex = selectBestMedia(selectedMedias);

        rtpConfig.remotePort = selectedMedias[mediaIndex].port;

        //update remote SIP-URI
        updateNetworkConfig(nullptr, nullptr, remoteUA);
        ohmcomm::info("SIP") << "Our INVITE was accepted, initializing communication" << ohmcomm::endl;

        //start communication
        startCommunication(selectedMedias[mediaIndex], rtpConfig, SDPMessageHandler::readRTCPAttribute(sdp));
    }
    else
    {
        ohmcomm::warn("SIP") << "Currently not supported: " << requestCommand << ohmcomm::endl;
        ohmcomm::warn("SIP") << "Please file a bug report at: " << OHMCOMM_HOMEPAGE << ohmcomm::endl;
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
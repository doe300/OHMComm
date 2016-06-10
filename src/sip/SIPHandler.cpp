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

SIPHandler::SIPHandler(const ohmcomm::NetworkConfiguration& sipConfig, const std::string& remoteUser, const AddUserFunction addUserFunction) : 
        userAgents(std::to_string(rand())), network(new ohmcomm::network::MulticastNetworkWrapper(sipConfig)), sipConfig(sipConfig), addUserFunction(addUserFunction), buffer(SIP_BUFFER_SIZE), state(SessionState::UNKNOWN)
{
    userAgents.thisUA.userName = ohmcomm::Utility::getUserName();
    userAgents.thisUA.hostName = ohmcomm::Utility::getDomainName();
    //we need an initial value for the local IP-address for the ";received="-tag
    userAgents.thisUA.ipAddress = ohmcomm::Utility::getLocalIPAddress(ohmcomm::Utility::getNetworkType(sipConfig.remoteIPAddress));
    userAgents.thisUA.tag = std::to_string(rand());
    userAgents.thisUA.port = sipConfig.localPort;
    userAgents.thisUA.callID = SIPHandler::generateCallID(ohmcomm::Utility::getDomainName());
    SIPUserAgent& initialRemoteUA = userAgents.getRemoteUA();
    initialRemoteUA.userName = remoteUser;
    initialRemoteUA.ipAddress = sipConfig.remoteIPAddress;
    initialRemoteUA.port = sipConfig.remotePort;
    updateNetworkConfig(nullptr, nullptr, initialRemoteUA);
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

void SIPHandler::onRemoteConnected(const unsigned int ssrc, const std::string& address, const unsigned short port)
{
    //associate SIP user-data with RTP-participant
    SIPUserAgent* agent = userAgents.findForAddress(address, port);
    if(agent != nullptr)
    {
        agent->associatedSSRC = ssrc;
    }
}

void SIPHandler::onRemoteRemoved(const unsigned int ssrc)
{
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
    state = SessionState::UNKNOWN;

    //how to determine if initiating or receiving side??
    //doesn't really matter, if we are the first to start, the other side won't receive our INVITE, otherwise we INVITE
    sendInviteRequest(userAgents.getRemoteUA());

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

std::string SIPHandler::generateCallID(const std::string& host)
{
    //Call-ID: UUID@host
    return (ohmcomm::Utility::generateRandomUUID() + "@") +host;
}

void SIPHandler::sendInviteRequest(SIPUserAgent& remoteUA)
{
    SIPRequestHeader header;
    header.requestCommand = SIP_REQUEST_INVITE;
    header.requestURI = remoteUA.getSIPURI();
    initializeHeaderFields(SIP_REQUEST_INVITE, header, nullptr, remoteUA);
    //header-fields
    header[SIP_HEADER_ALLOW] = SIP_ALLOW_METHODS;
    header[SIP_HEADER_ACCEPT] = SIP_ACCEPT_TYPES;
    header[SIP_HEADER_CONTENT_TYPE] = MIME_SDP;

    NetworkConfiguration rtpConfig = sipConfig;
    rtpConfig.localPort = DEFAULT_NETWORK_PORT;
    rtpConfig.remotePort = DEFAULT_NETWORK_PORT;
    const std::string messageBody = SDPMessageHandler::createSessionDescription(userAgents.thisUA.userName, rtpConfig);

    const std::string message = SIPPackageHandler::createRequestPackage(header, messageBody);
    ohmcomm::info("SIP") << "Sending INVITE to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    
    state = SessionState::INVITING;
}

void SIPHandler::sendCancelRequest(SIPUserAgent& remoteUA)
{
    SIPRequestHeader header(SIP_REQUEST_CANCEL, remoteUA.getSIPURI());
    //branch must be the same as on INVITE
    const std::string retainLastBranch(remoteUA.lastBranch);
    initializeHeaderFields(SIP_REQUEST_CANCEL, header, nullptr, remoteUA);
    //need to manually overwrite CSeq to use the value from INVITE (previous request)
    --remoteUA.sequenceNumber;
    header[SIP_HEADER_CSEQ] = (std::to_string(remoteUA.sequenceNumber)+" ") + SIP_REQUEST_CANCEL;
    //need to manually set the branch-tag to the value from the INVITE
    header[SIP_HEADER_VIA].replace(header[SIP_HEADER_VIA].find_last_of('=') + 1, header.getBranchTag().size(), retainLastBranch);
    
    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    ohmcomm::info("SIP") << "Sending CANCEL to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    
    state = SessionState::SHUTDOWN;
}

void SIPHandler::sendOptionsResponse(SIPUserAgent& remoteUA, const SIPRequestHeader* requestHeader)
{
    SIPResponseHeader responseHeader(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    initializeHeaderFields("", responseHeader, requestHeader, remoteUA);
    
    //RFC 3261, Section 11.2: "Allow, Accept, Accept-Encoding, Accept-Language, and Supported header fields SHOULD be present[...]"
    responseHeader[SIP_HEADER_ALLOW] = SIP_ALLOW_METHODS;
    responseHeader[SIP_HEADER_ACCEPT] = SIP_ACCEPT_TYPES;
    responseHeader[SIP_HEADER_SUPPORTED] = SIP_SUPPORTED_FIELDS;
    //RFC 3261, Section 20.2: "If no Accept-Encoding header field is present, the server SHOULD assume a default value of identity."
    //RFC 3261, Section 20.3: "If no Accept-Language header field is present, the server SHOULD assume all languages are acceptable to the client."
    
    //RFC 3840, Section 8: "[...] add feature parameters to the Contact header field in the OPTIONS [...]"
    responseHeader[SIP_HEADER_CONTACT] += SIP_CAPABILITIES;
    
    //RFC 3261, Section 11.2: "A message body MAY be sent, the type of which is determined by the Accept header field 
    // in the OPTIONS request (application/sdp is the default if the Accept header field is not present). If the types
    // include one that can describe media capabilities, the UAS SHOULD include a body in the response for that purpose"
    std::string messageBody("");
    if((*(requestHeader))[SIP_HEADER_CONTENT_TYPE].compare(MIME_SDP) == 0)
    {
        //add SDP message body
        responseHeader[SIP_HEADER_CONTENT_TYPE] = MIME_SDP;
        NetworkConfiguration rtpConfig = sipConfig;
        rtpConfig.localPort = DEFAULT_NETWORK_PORT;
        rtpConfig.remotePort = DEFAULT_NETWORK_PORT;
        messageBody = SDPMessageHandler::createSessionDescription(userAgents.thisUA.userName, rtpConfig);
    }
    
    const std::string message = SIPPackageHandler::createResponsePackage(responseHeader, messageBody);
    ohmcomm::info("SIP") << "Sending OPTIONS response to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
}

void SIPHandler::sendResponse(const unsigned int responseCode, const std::string reasonPhrase, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA)
{
    SIPResponseHeader header(responseCode, reasonPhrase);
    initializeHeaderFields("", header, requestHeader, remoteUA);
    const std::string message = SIPPackageHandler::createResponsePackage(header, "");
    network->sendData(message.data(), message.size());
}

void SIPHandler::sendByeRequest(SIPUserAgent& remoteUA)
{
    SIPRequestHeader header(SIP_REQUEST_BYE, remoteUA.getSIPURI());
    initializeHeaderFields(SIP_REQUEST_BYE, header, nullptr, remoteUA);

    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    ohmcomm::info("SIP") << "Sending BYE to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    
    state = SessionState::SHUTDOWN;
}

void SIPHandler::sendAckRequest(SIPUserAgent& remoteUA)
{
    SIPRequestHeader header(SIP_REQUEST_ACK, remoteUA.getSIPURI());
    initializeHeaderFields(SIP_REQUEST_ACK, header, nullptr, remoteUA);

    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    network->sendData(message.data(), message.size());
}

void SIPHandler::sendRegisterRequest(SIPUserAgent& registerUA)
{
    //RFC 3261, Section 10.2: 
    //"The "userinfo" and "@" components of the SIP URI MUST NOT be present"
    SIPRequestHeader header(SIP_REQUEST_REGISTER, SIPGrammar::toSIPURI({"sip", "", "", registerUA.hostName.empty() ? registerUA.ipAddress : registerUA.hostName, registerUA.port}));
    initializeHeaderFields(SIP_REQUEST_REGISTER, header, nullptr, registerUA);
    
    header[SIP_HEADER_ALLOW] = SIP_ALLOW_METHODS;
    header[SIP_HEADER_ACCEPT] = SIP_ACCEPT_TYPES;
    header[SIP_HEADER_SUPPORTED] = SIP_SUPPORTED_FIELDS;
    header[SIP_HEADER_CONTACT] += SIP_CAPABILITIES;
    
    ohmcomm::info("SIP") << "Sending REGISTER to " << registerUA.getSIPURI() << ohmcomm::endl;
    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    network->sendData(message.data(), message.size());
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
                    state = SessionState::UNKNOWN;
                    shutdown();
                    return;
                }
                //2.2 send ok to start communication (with selected media-format)
                SIPResponseHeader responseHeader(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
                initializeHeaderFields(SIP_REQUEST_INVITE, responseHeader, &requestHeader, remoteUA);
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
        else if (SIP_REQUEST_BYE.compare(requestHeader.requestCommand) == 0)
        {
            ohmcomm::info("SIP") << "BYE received, shutting down ..." << ohmcomm::endl;
            //send ok to verify reception
            sendResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK, &requestHeader, remoteUA);
            //remove remote user-agent
            if(remoteUA.associatedSSRC >= 0)
            {
                ohmcomm::rtp::ParticipantDatabase::removeParticipant((uint32_t)remoteUA.associatedSSRC);
            }
            userAgents.removeRemoteUA(remoteUA.tag);
            //end communication, if established
            shutdownInternal();
            stopCallback();
        }
        else if(SIP_REQUEST_ACK.compare(requestHeader.requestCommand) == 0)
        {
            // remote has acknowledged our last message
            // we currently have no need to wait for confirmation
        }
        else if(SIP_REQUEST_CANCEL.compare(requestHeader.requestCommand) == 0)
        {
            //remote has canceled their INVITE
            //if we have already established communication - do nothing
            if(state != SessionState::ESTABLISHED)
            {
                ohmcomm::info("SIP") << "CANCEL received, canceling session setup ..." << ohmcomm::endl;
                //send ok to verify reception
                sendResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK, &requestHeader, remoteUA);
                userAgents.removeRemoteUA(remoteUA.tag);
                if(userAgents.getNumberOfRemotes() == 0)
                {
                    //our initial call failed, shut down
                    shutdownInternal();
                    stopCallback();
                }
            }
        }
        else if(SIP_REQUEST_OPTIONS.compare(requestHeader.requestCommand) == 0)
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
            
            sendOptionsResponse(remoteUA, &requestHeader);
        }
        else if(SIP_REQUEST_INFO.compare(requestHeader.requestCommand) == 0)
        {
            //RFC 2976, Section 2.2: "A 200 OK response MUST be sent by a UAS[...]"
            //until an extension to SIP is supported utilizing INFO, we create a simple response,
            //since there are no required fields for INFO responses as of RFC 2976
            sendResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK, &requestHeader, remoteUA);
            
            //NOTE: RFC 6068 extends INFO greatly but is not supported!
        }
        else
        {
            ohmcomm::warn("SIP") << "Received not implemented request-method: " << requestHeader.requestCommand << " " << requestHeader.requestURI << ohmcomm::endl;
            //by default, send method not allowed
            sendResponse(SIP_RESPONSE_NOT_IMPLEMENTED_CODE, SIP_RESPONSE_NOT_IMPLEMENTED, &requestHeader, remoteUA);
            state = SessionState::UNKNOWN;
        }
    }
    catch(const std::exception& error)
    {
        sendResponse(SIP_RESPONSE_SERVER_ERROR_CODE, SIP_RESPONSE_SERVER_ERROR, &requestHeader, remoteUA);
        ohmcomm::error("SIP") << "Server-error: " << error.what() << ohmcomm::endl;
        shutdown();
    }
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
        if(responseHeader.statusCode >= 100 && responseHeader.statusCode < 200)
        {
            //for all provisional status-codes - do nothing
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_OK_CODE)
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

void SIPHandler::initializeHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader, SIPUserAgent& remoteUA)
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

int SIPHandler::selectBestMedia(const std::vector<MediaDescription>& availableMedias) const
{
    if(availableMedias.empty())
    {
        return -1;
    }
    //we only have one media, select it
    if(availableMedias.size() == 1)
    {
        return 0;
    }
    //determine best media (best quality)
    //we achieve this by searching for the best quality for each of the supported format in descending priority
    for(const SupportedFormat& format : SupportedFormats::getFormats())
    {
        int index = -1;
        unsigned int sampleRate = 0;
        for(unsigned short i = 0; i < availableMedias.size(); i++)
        {
            if(ohmcomm::Utility::equalsIgnoreCase(format.encoding, availableMedias[i].encoding) && availableMedias[i].sampleRate > sampleRate)
            {
                index = i;
                sampleRate = availableMedias[i].sampleRate;
            }
        }
        if(index != -1)
        {
            return index;
        }
    }
    //if we come to this point, we couldn't match any media-format
    return -1;
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

SIPGrammar::SIPURI SIPHandler::toSIPURI(const SIPUserAgent& sipUA, const bool withParameters)
{
    SIPGrammar::SIPURI uri{"sip", sipUA.userName, "", sipUA.hostName.empty() ? sipUA.ipAddress : sipUA.hostName, sipUA.port};
    if(withParameters && !sipUA.tag.empty())
    {
        uri.parameters["tag"] = sipUA.tag;
    }
    return uri;
}

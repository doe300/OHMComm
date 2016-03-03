/* 
 * File:   SIPHandler.cpp
 * Author: daniel
 * 
 * Created on November 26, 2015, 12:37 PM
 */

#include <algorithm>

#include "sip/SIPHandler.h"
#include "network/UDPWrapper.h"

const std::string SIPHandler::SIP_ALLOW_METHODS = Utility::joinStrings({SIP_REQUEST_INVITE, SIP_REQUEST_ACK, SIP_REQUEST_BYE, SIP_REQUEST_CANCEL}, " ");
const std::string SIPHandler::SIP_ACCEPT_TYPES = Utility::joinStrings({MIME_SDP, MIME_MULTIPART_MIXED, MIME_MULTIPART_ALTERNATIVE}, ", ");

SIPHandler::SIPHandler(const NetworkConfiguration& sipConfig, const std::string& remoteUser, const std::function<void(const MediaDescription, const NetworkConfiguration, const NetworkConfiguration)> configFunction) : 
        userAgents(std::to_string(rand())), network(new UDPWrapper(sipConfig)), sipConfig(sipConfig), configFunction(configFunction), buffer(SIP_BUFFER_SIZE), state(SessionState::UNKNOWN)
{
    userAgents.thisUA.userName = Utility::getUserName();
    userAgents.thisUA.hostName = Utility::getDomainName();
    //we need an initial value for the local IP-address for the ";received="-tag
    userAgents.thisUA.ipAddress = Utility::getLocalIPAddress(Utility::getNetworkType(sipConfig.remoteIPAddress));
    userAgents.thisUA.tag = std::to_string(rand());
    userAgents.thisUA.port = sipConfig.localPort;
    userAgents.thisUA.callID = SIPHandler::generateCallID(Utility::getDomainName());
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
    std::cout << "SIP-Handler started ..." << std::endl;
    state = SessionState::UNKNOWN;

    //how to determine if initiating or receiving side??
    //doesn't really matter, if we are the first to start, the other side won't receive our INVITE, otherwise we INVITE
    sendInviteRequest(userAgents.getRemoteUA());

    while (threadRunning)
    {
        //wait for package and store it in the SIPPackageHandler
        const NetworkWrapper::Package result = network->receiveData(buffer.data(), buffer.size());
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
    std::cout << "SIP-Handler shut down!" << std::endl;
}

std::string SIPHandler::generateCallID(const std::string& host)
{
    //TODO generate UUID
    //use or copy: https://github.com/graeme-hill/crossguid/blob/master/guid.cpp
    //Call-ID: UUID@host
    return (std::to_string(std::rand()) + "@") +host;
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
    std::cout << "SIP: Sending INVITE to " << remoteUA.getSIPURI() << std::endl;
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
    std::cout << "SIP: Sending CANCEL to " << remoteUA.getSIPURI() << std::endl;
    network->sendData(message.data(), message.size());
    
    state = SessionState::SHUTDOWN;
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
    std::cout << "SIP: Sending BYE to " << remoteUA.getSIPURI() << std::endl;
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

void SIPHandler::handleSIPRequest(const void* buffer, unsigned int packageLength, const NetworkWrapper::Package& packageInfo)
{
    SIPRequestHeader requestHeader;
    std::string requestBody;
    SIPUserAgent& remoteUA = userAgents.getRemoteUA(/*XXXrequestHeader.getRemoteTag()*/);
    try
    {
        requestBody = SIPPackageHandler::readRequestPackage(buffer, packageLength, requestHeader);
        std::cout << "SIP: Request received: " << requestHeader.requestCommand << " " << requestHeader.requestURI << std::endl;
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
        std::cout << "SIP: Received bad request: " << error.what() << std::endl;
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
                std::cout << "SIP: Received INVITE, but communication already active, answering: " << SIP_RESPONSE_BUSY << std::endl;
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
                        std::cout << "SIP: Received multi-part INVITE without SDP body!" << std::endl;
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
                NetworkConfiguration rtpConfig;
                rtpConfig.remoteIPAddress = sdp.getConnectionAddress();
                rtpConfig.localPort = DEFAULT_NETWORK_PORT;
                rtpConfig.remotePort = availableMedias[bestMediaIndex].port;
                const std::string messageBody = SDPMessageHandler::createSessionDescription(userAgents.thisUA.userName, rtpConfig, {availableMedias[bestMediaIndex]});
                const std::string message = SIPPackageHandler::createResponsePackage(responseHeader, messageBody);

                std::cout << "SIP: Accepting INVITE from " << requestHeader[SIP_HEADER_CONTACT] << std::endl;
                network->sendData(message.data(), message.size());

                //start communication
                startCommunication(availableMedias[bestMediaIndex], rtpConfig, SDPMessageHandler::readRTCPAttribute(sdp));
            }
            else
            {
                std::cout << "SIP: Received INVITE without SDP body!" << std::endl;
                if(requestHeader.getContentLength() > 0)
                {
                    std::cout << requestHeader.getContentLength() << " " << requestHeader[SIP_HEADER_CONTENT_TYPE] << std::endl;
                }
                sendResponse(SIP_RESPONSE_NOT_ACCEPTABLE_CODE, SIP_RESPONSE_NOT_ACCEPTABLE, &requestHeader, remoteUA);
            }
        }
        else if (SIP_REQUEST_BYE.compare(requestHeader.requestCommand) == 0)
        {
            std::cout << "SIP: BYE received, shutting down ..." << std::endl;
            //send ok to verify reception
            sendResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK, &requestHeader, remoteUA);
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
                std::cout << "SIP: CANCEL received, canceling session setup ..." << std::endl;
                //send ok to verify reception
                sendResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK, &requestHeader, remoteUA);
                shutdownInternal();
                stopCallback();
            }
        }
        else
        {
            std::cout << "SIP: Received not implemented request-method: " << requestHeader.requestCommand << " " << requestHeader.requestURI << std::endl;
            //by default, send method not allowed
            sendResponse(SIP_RESPONSE_NOT_IMPLEMENTED_CODE, SIP_RESPONSE_NOT_IMPLEMENTED, &requestHeader, remoteUA);
            state = SessionState::UNKNOWN;
        }
    }
    catch(const std::exception& error)
    {
        sendResponse(SIP_RESPONSE_SERVER_ERROR_CODE, SIP_RESPONSE_SERVER_ERROR, &requestHeader, remoteUA);
        std::cerr << "SIP: Server-error: " << error.what() << std::endl;
        shutdown();
    }
}

void SIPHandler::handleSIPResponse(const void* buffer, unsigned int packageLength, const NetworkWrapper::Package& packageInfo)
{
    SIPResponseHeader responseHeader;
    std::string responseBody;
    try
    {
        responseBody = SIPPackageHandler::readResponsePackage(buffer, packageLength, responseHeader);
        std::cout << "SIP: Response received: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << std::endl;
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
        std::cout << "SIP: Received bad response: " << error.what() << std::endl;
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
                        std::cout << "SIP: Received multi-part OK without SDP body!" << std::endl;
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
                    std::cerr << "SIP: Could not agree on audio-configuration, aborting!" << std::endl;
                    shutdownInternal();
                }
                //support for SDP offer/answer model (RFC 3264)
                //if medias > 1, select best media-type
                mediaIndex = selectBestMedia(selectedMedias);

                rtpConfig.remotePort = selectedMedias[mediaIndex].port;

                //update remote SIP-URI
                updateNetworkConfig(nullptr, nullptr, remoteUA);
                std::cout << "SIP: Our INVITE was accepted, initializing communication" << std::endl;

                //start communication
                startCommunication(selectedMedias[mediaIndex], rtpConfig, SDPMessageHandler::readRTCPAttribute(sdp));
            }
            else
            {
                std::cout << "Currently not supported: " << requestCommand << std::endl;
                std::cout << "SIP: Please file a bug report at: " << OHMCOMM_HOMEPAGE << std::endl;
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
            std::cout << "SIP: We sent bad request: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << std::endl;
            std::cout << "SIP: Please file a bug report at: " << OHMCOMM_HOMEPAGE << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_UNAUTHORIZED_CODE || responseHeader.statusCode == SIP_RESPONSE_PROXY_AUTHENTICATION_REQUIRED_CODE)
        {
            std::cout << "SIP: Server requires authorization!" << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_FORBIDDEN_CODE)
        {
            std::cout << "SIP: Access forbidden!" << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_NOT_FOUND_CODE)
        {
            std::cout << "SIP: Request-URI not found!" << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_METHOD_NOT_ALLOWED_CODE)
        {
            std::cout << "SIP: Server didn't allow our request-method!" << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_NOT_ACCEPTABLE_CODE || responseHeader.statusCode == SIP_RESPONSE_NOT_ACCEPTABLE_HERE_CODE)
        {
            std::cout << "SIP: Request could not be accepted: " << responseHeader.reasonPhrase << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_REQUEST_TIMEOUT_CODE || responseHeader.statusCode == SIP_RESPONSE_GONE_CODE)
        {
            std::cout << "SIP: Could not reach communication partner!" << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_REQUEST_ENTITY_TOO_LARGE_CODE || responseHeader.statusCode == SIP_RESPONSE_REQUEST_URI_TOO_LONG_CODE
                || responseHeader.statusCode == SIP_RESPONSE_UNSUPPORTED_MEDIA_TYPE_CODE || responseHeader.statusCode == SIP_RESPONSE_UNSUPPORTED_SCHEME_CODE)
        {
            std::cout << "SIP: Remote could not handle our request:" << responseHeader.statusCode << " " << responseHeader.reasonPhrase << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_EXTENSION_REQUIRED_CODE)
        {
            std::cout << "SIP: Remote requires an extension, we do not support!" << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_TEMPORARILY_UNAVAILABLE_CODE)
        {
            std::cout << "SIP: Communication partner temporarily unavailable: " << responseHeader.reasonPhrase << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_LOOP_DETECTED_CODE || responseHeader.statusCode == SIP_RESPONSE_TOO_MANY_HOPS_CODE)
        {
            std::cout << "SIP: Network-error: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_BUSY_CODE || responseHeader.statusCode == SIP_RESPONSE_BUSY_EVERYWHERE_CODE
                || responseHeader.statusCode == SIP_RESPONSE_DECLINE_CODE)
        {
            //remote is busy or call was declined
            std::cout << "SIP: Could not establish connection: " << responseHeader.reasonPhrase << std::endl;
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
            std::cout << "SIP: Remote server error: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << std::endl;
            shutdown();
        }
        else if(responseHeader.statusCode == SIP_RESPONSE_DOES_NOT_EXIST_ANYWHERE_CODE)
        {
            std::cout << "SIP: Requested user does not exist!" << std::endl;
            shutdown();
        }
        else
        {
            std::cout << "SIP: Received unsupported response: " << responseHeader.statusCode << " " << responseHeader.reasonPhrase << std::endl;
            std::cout << "SIP: Please file a bug report at: " << OHMCOMM_HOMEPAGE << std::endl;
        }
    }
    catch(const std::exception& error)
    {
        sendResponse(SIP_RESPONSE_SERVER_ERROR_CODE, SIP_RESPONSE_SERVER_ERROR, nullptr, remoteUA);
        std::cerr << "SIP: Server-error: " << error.what() << std::endl;
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
    
    header[SIP_HEADER_CONTACT] = (userAgents.thisUA.userName + " <") + userAgents.thisUA.getSIPURI() + ">";
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
        reqHeader[SIP_HEADER_TO] = ((remoteUA.userName + " <") + remoteUA.getSIPURI() + ">") + (remoteUA.tag.empty() ? std::string("") : (std::string(";tag=") + remoteUA.tag));
        reqHeader[SIP_HEADER_FROM] = (((userAgents.thisUA.userName + " <") + userAgents.thisUA.getSIPURI() + ">") + ";tag=") + userAgents.thisUA.tag;
    }
    
    catch(const std::bad_cast& e)
    {
        //do nothing
    }
    try
    {
        SIPResponseHeader& resHeader = dynamic_cast<SIPResponseHeader&>(header);
        //CSeq of response if the same as request
        resHeader[SIP_HEADER_CSEQ] = requestHeader->operator[](SIP_HEADER_CSEQ);
        //From/To has the value from the request
        if(requestHeader->operator [](SIP_HEADER_TO).find("tag=") == std::string::npos)
        {
            //on initial response, add tag
            resHeader[SIP_HEADER_TO] = (requestHeader->operator[](SIP_HEADER_TO) + ";tag=") + userAgents.thisUA.tag;
        }
        else
        {
            resHeader[SIP_HEADER_TO] = requestHeader->operator[](SIP_HEADER_TO);
        }
        resHeader[SIP_HEADER_FROM] = requestHeader->operator[](SIP_HEADER_FROM);
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
            if(Utility::equalsIgnoreCase(format.encoding, availableMedias[i].encoding) && availableMedias[i].sampleRate > sampleRate)
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

void SIPHandler::updateNetworkConfig(const SIPHeader* header, const NetworkWrapper::Package* packageInfo, SIPUserAgent& remoteUA)
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
            const auto socketAddress = Utility::getSocketAddress(&(packageInfo->ipv6Address), sizeof(packageInfo->ipv6Address), packageInfo->isIPv6);
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
            std::cerr << "No address found for host: " << remoteAddress.uri.host << std::endl;
            throw std::invalid_argument("Invalid IP address!");
        }
    }
    //check if configuration has changed
    if(sipConfig.remotePort != remoteUA.port || remoteUA.ipAddress.compare(sipConfig.remoteIPAddress) != 0)
    {
        std::cout << "Reconnecting SIP ..." << std::endl;
        //reset network-wrapper to send new packages to correct address
        sipConfig.remoteIPAddress = remoteUA.ipAddress;
        sipConfig.remotePort = remoteUA.port;
        network->closeNetwork();
        std::cout << "Connecting to: " << sipConfig.remoteIPAddress << ':' << sipConfig.remotePort << std::endl;
        //wait for socket to be closed
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        network.reset(new UDPWrapper(sipConfig));
        
        //update all configuration-dependant values
        userAgents.thisUA.ipAddress = Utility::getLocalIPAddress(Utility::getNetworkType(sipConfig.remoteIPAddress));
    }
}

void SIPHandler::startCommunication(const MediaDescription& descr, const NetworkConfiguration& rtpConfig, const NetworkConfiguration rtcpConfig)
{
    state = SessionState::ESTABLISHED;
    std::cout << "SIP: Using following SDP configuration: "
            << descr.protocol << " " << descr.payloadType << " " << descr.encoding << '/' << descr.sampleRate << '/' << descr.numChannels
            << std::endl;
    if(rtcpConfig.remotePort != 0)
    {
        if(!rtcpConfig.remoteIPAddress.empty())
        {
            std::cout << "SIP: Using custom remote RTCP address: " << rtcpConfig.remoteIPAddress << ":" << rtcpConfig.remotePort << std::endl;
        }
        else
        {
            std::cout << "SIP: Using custom remote RTCP port: " << rtcpConfig.remotePort << std::endl;
        }
    }
    configFunction(descr, rtpConfig, rtcpConfig);
}

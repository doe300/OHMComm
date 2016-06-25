/* 
 * File:   SIPRequest.cpp
 * Author: doe300
 * 
 * Created on June 12, 2016, 2:00 PM
 */

#include "sip/SIPRequest.h"

#include <typeinfo>

#include "sip/SIPHandler.h"
#include "Logger.h"
#include "config/ConfigurationMode.h"
#include "Parameters.h"

using namespace ohmcomm::sip;

void ohmcomm::sip::initializeSIPHeaderFields(const std::string& requestMethod, SIPHeader& header, const SIPRequestHeader* requestHeader, const SIPUserAgent& localUA, SIPUserAgent& remoteUA, const unsigned short localPort)
{
    //mandatory header-fields:
    //To, From, CSeq, Call-ID, Max-Forwards, Via, Contact
    header.fields.clear();
    //"rport" is specified in RFC3581 and requests the response to be sent to the originating port
    //"branch"-tag unique for all requests, randomly generated, starting with "z9hG4bK", ACK has same as INVITE (for non-2xx ACK) -> 8.1.1.7 Via
    //"received"-tag has the IP of the receiving endpoint
    remoteUA.lastBranch = requestHeader != nullptr ? requestHeader->getBranchTag() : (std::string("z9hG4bK") + std::to_string(rand()));
    const std::string receivedTag = requestHeader != nullptr ? std::string(";received=") + localUA.ipAddress : "";
    header[SIP_HEADER_VIA] = ((SIP_VERSION + "/UDP ") + localUA.hostName + ":") + ((((std::to_string(localPort)
                             + ";rport") + receivedTag) + ";branch=") + remoteUA.lastBranch);
    //TODO for response, copy/retain VIA-fields of request (multiple!)
    header[SIP_HEADER_CONTACT] = SIPGrammar::toNamedAddress(toSIPURI(localUA, false), localUA.userName);
    //if we INVITE, the remote call-ID is empty and we use ours, otherwise use the remote's
    header[SIP_HEADER_CALL_ID] =  remoteUA.callID.empty() ? localUA.callID : remoteUA.callID;
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
        if(SIP_REQUEST_REGISTER.compare(requestMethod) == 0)
        {
            //for REGISTER the FROM header-field uses the address of the remote UAS
            SIPUserAgent dummy = remoteUA;
            dummy.tag = localUA.tag;
            reqHeader[SIP_HEADER_FROM] = SIPGrammar::toNamedAddress(toSIPURI(dummy, true), dummy.userName);
        }
        else
        {
            reqHeader[SIP_HEADER_FROM] = SIPGrammar::toNamedAddress(toSIPURI(localUA, true), localUA.userName);
        }
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
            resHeader[SIP_HEADER_TO] = ((*requestHeader)[SIP_HEADER_TO] + ";tag=") + localUA.tag;
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
    header[SIP_HEADER_USER_AGENT] = std::string("OHMComm/") + ohmcomm::OHMCOMM_VERSION;
}

SIPGrammar::SIPURI ohmcomm::sip::toSIPURI(const SIPUserAgent& sipUA, const bool withParameters)
{
    SIPGrammar::SIPURI uri{"sip", sipUA.userName, "", sipUA.hostName.empty() ? sipUA.ipAddress : sipUA.hostName, sipUA.port};
    if(withParameters && !sipUA.tag.empty())
    {
        uri.parameters["tag"] = sipUA.tag;
    }
    return uri;
}

SIPRequest::SIPRequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network) :
    thisUA(thisUA), requestHeader(requestHeader), remoteUA(remoteUA), localPort(localPort), network(network)
{
}

SIPRequest::~SIPRequest()
{
}

bool SIPRequest::isMatchingResponse(const SIPResponseHeader& header) const
{
    return header[SIP_HEADER_CSEQ].compare(requestHeader[SIP_HEADER_CSEQ]) == 0;
}

void SIPRequest::sendSimpleResponse(const unsigned int responseCode, const std::string reasonPhrase)
{
    SIPResponseHeader header(responseCode, reasonPhrase);
    initializeSIPHeaderFields("", header, &requestHeader, thisUA, remoteUA, localPort);
    const std::string message = SIPPackageHandler::createResponsePackage(header, "");
    network->sendData(message.data(), message.size());
}


//
// REGISTER
//

REGISTERRequest::REGISTERRequest(const SIPUserAgent& thisUA, SIPUserAgent& registerUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network, const std::string& userName, const std::string& password, const unsigned short expiresInSeconds) :
    SIPRequest(thisUA, {}, registerUA, localPort, network), expiresInSeconds(expiresInSeconds), userName(userName), password(password)
{
}


REGISTERRequest::~REGISTERRequest()
{

}

bool REGISTERRequest::sendRequest(const std::string& requestBody)
{
    requestHeader = createRequest();
    
    ohmcomm::info("SIP") << "Sending REGISTER to " << remoteUA.getSIPURI() << ohmcomm::endl;
    const std::string message = SIPPackageHandler::createRequestPackage(requestHeader, requestBody);
    network->sendData(message.data(), message.size());
    return true;
}

bool REGISTERRequest::handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    if(responseHeader.statusCode == SIP_RESPONSE_UNAUTHORIZED_CODE)
    {
        ohmcomm::info("SIP") << "Server requires authorization!" << ohmcomm::endl;
        
        if(authentication)
        {
            //there already is an authentication-method, so previous authentication-attempts failed
            ohmcomm::warn("SIP") << "Previous authorization failed!" << ohmcomm::endl;
        }
        authentication = Authentication::getAuthenticationMethod(requestHeader.requestCommand, requestHeader.requestURI, responseHeader[SIP_HEADER_WWW_AUTHENTICATE]);
        if(authentication)
        {
            ohmcomm::info("SIP") << "Trying to authenticate..." << ohmcomm::endl;
            requestHeader = createRequest();
            //re-send request with new authentication
            requestHeader[SIP_HEADER_AUTHORIZATION] = authentication->createAuthenticationHeader(userName, password);
            //set expires header-field
            requestHeader[SIP_HEADER_EXPIRES] = std::to_string(expiresInSeconds);
            
            ohmcomm::info("SIP") << "Re-sending REGISTER to " << remoteUA.getSIPURI() << ohmcomm::endl;
            const std::string message = SIPPackageHandler::createRequestPackage(requestHeader, "");
            network->sendData(message.data(), message.size());
            return true;
        }
        else
        {
            ohmcomm::warn("SIP") << "No authentication provided, can't register!" << ohmcomm::endl;
        }
        return true;
    }
    else if(responseHeader.statusCode == SIP_RESPONSE_OK_CODE)
    {
        if(authentication)
        {
            //TODO update local URI with contact
            const std::string& contactHeader = responseHeader[SIP_HEADER_CONTACT];
            std::string::size_type i;
            int expiresSeconds = 0;
            if((i = contactHeader.find(";expires=")) != std::string::npos)
            {
                std::string expiresValue = contactHeader.substr(i + std::string(";expires=").size());
                expiresValue = expiresValue.substr(0, expiresValue.find_first_of("; \r\n"));
                expiresSeconds = atoi(expiresValue.data());
            }
            else
            {
                expiresSeconds = atoi(responseHeader[SIP_HEADER_EXPIRES].data());
            }
            authentication->setAuthenticated(std::chrono::system_clock::now() + std::chrono::seconds(expiresSeconds));
        }
        return true;
    }
    //any other response is currently not handled
    return false;
}

bool REGISTERRequest::handleRequest(const std::string& requestBody)
{
    //library doesn't implement the role of an UAC
    sendSimpleResponse(SIP_RESPONSE_NOT_IMPLEMENTED_CODE, SIP_RESPONSE_NOT_IMPLEMENTED);
    ohmcomm::info("SIP") << "Cannot handle REGISTER requests!" << ohmcomm::endl;
    return true;
}

bool REGISTERRequest::isCompleted() const
{
    return authentication && authentication->isAuthenticated();
}

std::unique_ptr<Authentication> REGISTERRequest::getAuthentication()
{
    std::unique_ptr<Authentication> tmp(nullptr);
    tmp.swap(authentication);
    return tmp;
}

SIPRequestHeader REGISTERRequest::createRequest() const
{
    //RFC 3261, Section 10.2: 
    //"The "userinfo" and "@" components of the SIP URI MUST NOT be present"
    SIPRequestHeader header(SIP_REQUEST_REGISTER, SIPGrammar::toSIPURI({"sip", "", "", remoteUA.hostName.empty() ? remoteUA.ipAddress : remoteUA.hostName, remoteUA.port}));
    initializeSIPHeaderFields(SIP_REQUEST_REGISTER, header, nullptr, thisUA, remoteUA, localPort);
    
    header[SIP_HEADER_ALLOW] = SIP_ALLOW_METHODS;
    header[SIP_HEADER_ACCEPT] = SIP_ACCEPT_TYPES;
    //header[SIP_HEADER_SUPPORTED] = SIP_SUPPORTED_FIELDS;
    header[SIP_HEADER_CONTACT] += SIP_CAPABILITIES;
    
    return header;
}

//
// OPTIONS
//

OPTIONSRequest::OPTIONSRequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network) :
SIPRequest(thisUA, requestHeader, remoteUA, localPort, network)
{
}

OPTIONSRequest::~OPTIONSRequest()
{
}

bool OPTIONSRequest::sendRequest(const std::string& requestBody)
{
    SIPRequestHeader header(SIP_REQUEST_OPTIONS, remoteUA.getSIPURI());
    initializeSIPHeaderFields(SIP_REQUEST_OPTIONS, header, nullptr, thisUA, remoteUA, localPort);
    
    requestHeader = header;
    
    ohmcomm::info("SIP") << "Sending OPTIONS to " << remoteUA.getSIPURI() << ohmcomm::endl;
    const std::string message = SIPPackageHandler::createRequestPackage(header, requestBody);
    network->sendData(message.data(), message.size());
    return true;
}

bool OPTIONSRequest::handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    //this UAC sent an OPTIONS request and now handle the response
    //TODO
    return false;
}

bool OPTIONSRequest::handleRequest(const std::string& requestBody)
{
    SIPResponseHeader responseHeader(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    initializeSIPHeaderFields("", responseHeader, &requestHeader, thisUA, remoteUA, localPort);
    
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
    if(requestHeader[SIP_HEADER_CONTENT_TYPE].compare(MIME_SDP) == 0)
    {
        //add SDP message body
        responseHeader[SIP_HEADER_CONTENT_TYPE] = MIME_SDP;
        NetworkConfiguration rtpConfig;
        rtpConfig.localPort = DEFAULT_NETWORK_PORT;
        rtpConfig.remoteIPAddress = remoteUA.ipAddress;
        rtpConfig.remotePort = DEFAULT_NETWORK_PORT;
        messageBody = SDPMessageHandler::createSessionDescription(thisUA.userName, rtpConfig);
    }
    
    const std::string message = SIPPackageHandler::createResponsePackage(responseHeader, messageBody);
    ohmcomm::info("SIP") << "Sending OPTIONS response to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    return true;
}

bool OPTIONSRequest::isCompleted() const
{
    //can be canceled at any time
    return true;
}

//
// INFO
//

INFORequest::INFORequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network) :
SIPRequest(thisUA, requestHeader, remoteUA, localPort, network)
{
}

INFORequest::~INFORequest()
{
}

bool INFORequest::sendRequest(const std::string& requestBody)
{
    SIPRequestHeader header(SIP_REQUEST_INFO, remoteUA.getSIPURI());
    initializeSIPHeaderFields(SIP_REQUEST_INFO, header, nullptr, thisUA, remoteUA, localPort);
    
    requestHeader = header;
    
    ohmcomm::info("SIP") << "Sending INFO to " << remoteUA.getSIPURI() << ohmcomm::endl;
    const std::string message = SIPPackageHandler::createRequestPackage(header, requestBody);
    network->sendData(message.data(), message.size());
    return true;
}

bool INFORequest::handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    //unless required by some extension, simple ignore response
    return responseHeader.statusCode == SIP_RESPONSE_OK_CODE;
}

bool INFORequest::handleRequest(const std::string& requestBody)
{
    //RFC 2976, Section 2.2: "A 200 OK response MUST be sent by a UAS[...]"
    //until an extension to SIP is supported utilizing INFO, we create a simple response,
    //since there are no required fields for INFO responses as of RFC 2976
    ohmcomm::info("SIP") << "Sending INFO response to " << remoteUA.getSIPURI() << ohmcomm::endl;
    sendSimpleResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);

    //NOTE: RFC 6068 extends INFO greatly but is not supported!
    return true;
}

bool INFORequest::isCompleted() const
{
    //can be canceled at any time
    return true;
}

//
// BYE
//

BYERequest::BYERequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network) :
SIPRequest(thisUA, requestHeader, remoteUA, localPort, network)
{
}

BYERequest::~BYERequest()
{
}

bool BYERequest::sendRequest(const std::string& requestBody)
{
    SIPRequestHeader header(SIP_REQUEST_BYE, remoteUA.getSIPURI());
    initializeSIPHeaderFields(SIP_REQUEST_BYE, header, nullptr, thisUA, remoteUA, localPort);
    
    requestHeader = header;

    const std::string message = SIPPackageHandler::createRequestPackage(header, requestBody);
    ohmcomm::info("SIP") << "Sending BYE to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    return true;
}

bool BYERequest::handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    //any response to our BYE request confirms the remote has received it correctly
    return responseHeader.statusCode == SIP_RESPONSE_OK_CODE;
}

bool BYERequest::handleRequest(const std::string& requestBody)
{
    //remove remote user-agent
    if(remoteUA.associatedSSRC >= 0)
    {
        ohmcomm::rtp::ParticipantDatabase::removeParticipant((uint32_t)remoteUA.associatedSSRC);
    }
    //acknowledge the BYE with a 200 OK
    ohmcomm::info("SIP") << "Acknowledging BYE of " << remoteUA.getSIPURI() << ohmcomm::endl;
    sendSimpleResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    return true;
}

bool BYERequest::isCompleted() const
{
    //can be canceled at any time
    return true;
}

//
// CANCEL
//

CANCELRequest::CANCELRequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network) :
SIPRequest(thisUA, requestHeader, remoteUA, localPort, network)
{
}

CANCELRequest::~CANCELRequest()
{
}

bool CANCELRequest::sendRequest(const std::string& requestBody)
{
    SIPRequestHeader header(SIP_REQUEST_CANCEL, remoteUA.getSIPURI());
    //branch must be the same as on INVITE
    const std::string retainLastBranch(remoteUA.lastBranch);
    initializeSIPHeaderFields(SIP_REQUEST_CANCEL, header, nullptr, thisUA, remoteUA, localPort);
    //need to manually overwrite CSeq to use the value from the previous request
    --remoteUA.sequenceNumber;
    header[SIP_HEADER_CSEQ] = (std::to_string(remoteUA.sequenceNumber)+" ") + SIP_REQUEST_CANCEL;
    //need to manually set the branch-tag to the value from the INVITE
    header[SIP_HEADER_VIA].replace(header[SIP_HEADER_VIA].find_last_of('=') + 1, header.getBranchTag().size(), retainLastBranch);
    
    requestHeader = header;
    
    const std::string message = SIPPackageHandler::createRequestPackage(header, requestBody);
    ohmcomm::info("SIP") << "Sending CANCEL to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    return true;
}

bool CANCELRequest::handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    //TODO the remote canceled some previous action
    return false;
}

bool CANCELRequest::handleRequest(const std::string& requestBody)
{
    //RFC 3261, section: 16.10: "If a matching response context is found, the element MUST immediately return a 200 (OK) response to the CANCEL request"
    sendSimpleResponse(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    return true;
}

bool CANCELRequest::isCompleted() const
{
    //can be canceled at any time
    return true;
}

//
// INVITE
//

INVITERequest::INVITERequest(const SIPUserAgent& thisUA, const SIPRequestHeader requestHeader, SIPUserAgent& remoteUA, const unsigned short localPort, ohmcomm::network::NetworkWrapper* network, const SIPSession::SessionState state, const ConnectCallback callback) :
SIPRequest(thisUA, requestHeader, remoteUA, localPort, network), connectCallback(callback), state(state)
{
}

INVITERequest::~INVITERequest()
{
}

bool INVITERequest::sendRequest(const std::string& requestBody)
{
    SIPRequestHeader header;
    header.requestCommand = SIP_REQUEST_INVITE;
    header.requestURI = remoteUA.getSIPURI();
    initializeSIPHeaderFields(SIP_REQUEST_INVITE, header, nullptr, thisUA, remoteUA, localPort);
    //header-fields
    header[SIP_HEADER_ALLOW] = SIP_ALLOW_METHODS;
    header[SIP_HEADER_ACCEPT] = SIP_ACCEPT_TYPES;
    header[SIP_HEADER_CONTENT_TYPE] = MIME_SDP;
    
    requestHeader = header;

    const std::string message = SIPPackageHandler::createRequestPackage(header, requestBody);
    ohmcomm::info("SIP") << "Sending INVITE to " << remoteUA.getSIPURI() << ohmcomm::endl;
    network->sendData(message.data(), message.size());
    state = SIPSession::SessionState::INVITING;
    return true;
}

bool INVITERequest::handleResponse(const ohmcomm::network::SocketAddress& sourceAddress, const SIPResponseHeader& responseHeader, std::string& responseBody, SIPUserAgent& remoteUA)
{
    if(responseHeader.statusCode != SIP_RESPONSE_OK_CODE)
    {
        //all non-OK responses are handled via SIPHandler at the moment
        return false;
    }
    //we have success - acknowledge reception
    sendAckRequest();

    //adds support for multipart containing SDP (RFC 5621)
    if(SIPPackageHandler::hasMultipartBody(responseHeader))
    {
        //we only need the SDP part of the body
        responseBody = SIPPackageHandler::readMultipartBody(responseHeader, responseBody)[MIME_SDP];
        if(responseBody.empty())
        {
            ohmcomm::warn("SIP") << "Received multi-part OK without SDP body!" << ohmcomm::endl;
            state = SIPSession::SessionState::DISCONNECTED;
            return true;
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
        //TODO send CANCEL??
        state = SIPSession::SessionState::DISCONNECTED;
        return true;
    }
    //support for SDP offer/answer model (RFC 3264)
    //if medias > 1, select best media-type
    mediaIndex = SDPMessageHandler::selectBestMedia(selectedMedias);

    rtpConfig.remotePort = selectedMedias[mediaIndex].port;

    ohmcomm::info("SIP") << "Our INVITE was accepted, initializing communication" << ohmcomm::endl;

    //start communication
    connectCallback(selectedMedias[mediaIndex], rtpConfig, SDPMessageHandler::readRTCPAttribute(sdp));
    state = SIPSession::SessionState::ESTABLISHED;
    return true;
}

bool INVITERequest::handleRequest(const std::string& request_body)
{
    //if we get invited, the Call-ID is set by the remote UAS
    remoteUA.callID = requestHeader[SIP_HEADER_CALL_ID];
    std::string requestBody = request_body;
    //adds support for multipart containing SDP (RFC 5621)
    if(SIPPackageHandler::hasMultipartBody(requestHeader))
    {
        //we only need the SDP part of the body
        requestBody = SIPPackageHandler::readMultipartBody(requestHeader, requestBody)[MIME_SDP];
        if(requestBody.empty())
        {
            ohmcomm::warn("SIP") << "Received multi-part INVITE without SDP body!" << ohmcomm::endl;
        }
    }
    if((requestHeader[SIP_HEADER_CONTENT_TYPE].compare(MIME_SDP) != 0 && !SIPPackageHandler::hasMultipartBody(requestHeader)) || requestBody.empty())
    {
        ohmcomm::warn("SIP") << "Received INVITE without SDP body!" << ohmcomm::endl;
        if(requestHeader.getContentLength() > 0)
        {
            ohmcomm::warn("SIP") << requestHeader.getContentLength() << " " << requestHeader[SIP_HEADER_CONTENT_TYPE] << ohmcomm::endl;
        }
        sendSimpleResponse(SIP_RESPONSE_NOT_ACCEPTABLE_CODE, SIP_RESPONSE_NOT_ACCEPTABLE);
        state = SIPSession::SessionState::DISCONNECTED;
        return true;
    }
    //0. send ringing
    sendSimpleResponse(SIP_RESPONSE_RINGING_CODE, SIP_RESPONSE_RINGING);
    //1. send dialog established
    sendSimpleResponse(SIP_RESPONSE_DIALOG_ESTABLISHED_CODE, SIP_RESPONSE_DIALOG_ESTABLISHED);
    
    if (state == SIPSession::SessionState::ESTABLISHED)
    {
        //2.1 send busy here if communication is already running
        sendSimpleResponse(SIP_RESPONSE_BUSY_CODE, SIP_RESPONSE_BUSY);
        ohmcomm::info("SIP") << "Received INVITE, but communication already active, answering: " << SIP_RESPONSE_BUSY << ohmcomm::endl;
        return true;
    }
    
    //handle correct INVITE
    const SessionDescription sdp = SDPMessageHandler::readSessionDescription(requestBody);
    const std::vector<MediaDescription> availableMedias = SDPMessageHandler::readMediaDescriptions(sdp);
    //select best media
    int bestMediaIndex = SDPMessageHandler::selectBestMedia(availableMedias);
    if(bestMediaIndex < 0)
    {
        //no useful media found - send message (something about not-supported or similar)
        sendSimpleResponse(SIP_RESPONSE_NOT_ACCEPTABLE_CODE, SIP_RESPONSE_NOT_ACCEPTABLE);
        state = SIPSession::SessionState::DISCONNECTED;
        return true;
    }
    //2.2 send ok to start communication (with selected media-format)
    SIPResponseHeader responseHeader(SIP_RESPONSE_OK_CODE, SIP_RESPONSE_OK);
    initializeSIPHeaderFields(SIP_REQUEST_INVITE, responseHeader, &requestHeader, thisUA, remoteUA, localPort);
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
    const std::string messageBody = SDPMessageHandler::createSessionDescription(thisUA.userName, rtpConfig, {availableMedias[bestMediaIndex]});
    const std::string message = SIPPackageHandler::createResponsePackage(responseHeader, messageBody);

    ohmcomm::info("SIP") << "Accepting INVITE from " << requestHeader[SIP_HEADER_CONTACT] << ohmcomm::endl;
    network->sendData(message.data(), message.size());

    //start communication
    connectCallback(availableMedias[bestMediaIndex], rtpConfig, SDPMessageHandler::readRTCPAttribute(sdp));
    state = SIPSession::SessionState::ESTABLISHED;
    return true;
}

bool INVITERequest::isCompleted() const
{
    //is completed, when OKayed or somehow refused
    return state == SIPSession::SessionState::DISCONNECTED || state == SIPSession::SessionState::ESTABLISHED;
}

SIPSession::SessionState INVITERequest::getState() const
{
    return state;
}

void INVITERequest::sendAckRequest()
{
    SIPRequestHeader header(SIP_REQUEST_ACK, remoteUA.getSIPURI());
    initializeSIPHeaderFields(SIP_REQUEST_ACK, header, nullptr, thisUA, remoteUA, localPort);

    const std::string message = SIPPackageHandler::createRequestPackage(header, "");
    network->sendData(message.data(), message.size());
}

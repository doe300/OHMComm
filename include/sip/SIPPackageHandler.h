/* 
 * File:   SIPHandler.h
 * Author: daniel
 *
 * Created on July 26, 2015, 12:44 PM
 */

#ifndef SIPPACKAGEHANDLER_H
#define	SIPPACKAGEHANDLER_H

#include <string>
#include <vector>
#include <string.h>
#include <map>
#include <tuple>
#include <exception>

#include "KeyValuePairs.h"

////
// SIP values
////

/*!
 * SIP version string, 'SIP/2.0'
 */
const std::string SIP_VERSION("SIP/2.0");

/*!
 * The MIME multipart type for mixed content: multipart/mixed
 */
const std::string MIME_MULTIPART_MIXED("multipart/mixed");
/*!
 * The MIME multipart type for alternative representations of same content: multipart/alternative
 */
const std::string MIME_MULTIPART_ALTERNATIVE("multipart/alternative");

const std::string CRLF("\r\n");

////
// SIP Request-Commands
////

/*!
 * Invites a user to a call.
 * This command initiates the communication
 */
const std::string SIP_REQUEST_INVITE("INVITE");

/*!
 * Acknowledges the reception of an INVITE request.
 */
const std::string SIP_REQUEST_ACK("ACK");

/*!
 * Terminates a connection between users
 */
const std::string SIP_REQUEST_BYE("BYE");

/*!
 * Terminates a request or search for a user.
 * It is used if a client sends an INVITE and then changes its decision to invite the recipient.
 */
const std::string SIP_REQUEST_CANCEL("CANCEL");

/*!
 * Requests information about a client's capabilities.
 * This command is similar to the HTTP OPTIONS-command
 */
const std::string SIP_REQUEST_OPTIONS("OPTIONS");

/*!
 * Registers a user's current location
 */
const std::string SIP_REQUEST_REGISTER("REGISTER");

/*!
 * Used for mid-session signaling
 */
const std::string SIP_REQUEST_INFO("INFO");

////
// Header-Fields (assortment)
//  for a more complete list, see http://www.networksorcery.com/enp/protocol/sip.htm
////

/*!
 * If the Accept header field is not present, the server SHOULD assume a default MIME value of application/sdp. 
 * An empty Accept header field means that no formats are acceptable.
 */
const std::string SIP_HEADER_ACCEPT("Accept");

/*!
 * The Allow header field lists the set of methods supported by the UA generating the message.
 * 
 * All methods, including ACK and CANCEL, understood by the UA MUST be included in the list of methods in the Allow 
 * header field, when present.  The absence of an Allow header field MUST NOT be interpreted to mean that the UA 
 * sending the message supports no methods.   Rather, it implies that the UA is not providing any information on what 
 * methods it supports.
 * 
 * Supplying an Allow header field in responses to methods other than OPTIONS reduces the number of messages needed.
 */
const std::string SIP_HEADER_ALLOW("Allow");

/*!
 * The Call-ID header field uniquely identifies a particular invitation or all registrations of a particular client.
 * A single multimedia conference can give rise to several calls with different Call-IDs, for example, 
 * if a user invites a single individual several times to the same (long-running) conference. 
 * Call-IDs are case-sensitive and are simply compared byte-by-byte.
 */
const std::string SIP_HEADER_CALL_ID("Call-ID");

/*!A Contact header field value provides a URI whose meaning depends on the type of request or response it is in.
 * 
 * A Contact header field value can contain a display name, a URI with URI parameters, and header parameters.
 * 
 * This document defines the Contact parameters "q" and "expires". These parameters are only used when the Contact is 
 * present in a REGISTER request or response, or in a 3xx response.  Additional parameters may be defined in other specifications.
 * 
 * When the header field value contains a display name, the URI including all URI parameters is enclosed in "<" and ">".  
 * If no "<" and ">" are present, all parameters after the URI are header parameters, not URI parameters.  
 * The display name can be tokens, or a quoted string, if a larger character set is desired.
 * 
 * Even if the "display-name" is empty, the "name-addr" form MUST be used if the "addr-spec" contains a comma, semicolon, 
 * or question mark.  There may or may not be LWS between the display-name and the "<".
 * 
 * These rules for parsing a display name, URI and URI parameters, and
 * header parameters also apply for the header fields To and From.
 * 
 */
const std::string SIP_HEADER_CONTACT("Contact");

/*!
 * Indicates the size of the message-body, in decimal number of bytes, sent to the recipient.
 * Applications SHOULD use this field to indicate the size of the message-body to be transferred, 
 * regardless of the media type of the entity. If a stream-based protocol (such as TCP) is used as transport, 
 * the header field MUST be used. The size of the message-body does not include the CRLF separating header fields and body. 
 * Any Content-Length greater than or equal to zero is a valid value. If no body is present in a message, 
 * then the Content-Length header field value MUST be cleared to zero.
 */
const std::string SIP_HEADER_CONTENT_LENGTH("Content-Length");

/*!
 * Indicates the media type of the message-body sent to the recipient. This header field MUST be present if the body 
 * is not empty. If the body is empty, and a Content-Type header field is present, it indicates that the body of the 
 * specific type has zero length (for example, an empty audio file).
 */
const std::string SIP_HEADER_CONTENT_TYPE("Content-Type");

/*!
 * A CSeq header field in a request contains a single decimal sequence number and the request method.
 * The sequence number MUST be expressible as a 32-bit unsigned integer. The method part of CSeq is case-sensitive. 
 * The CSeq header field serves to order transactions within a dialog, to provide a means to uniquely identify 
 * transactions, and to differentiate between new requests and request retransmissions.
 * Two CSeq header fields are considered equal if the sequence number and the request method are identical.
 */
const std::string SIP_HEADER_CSEQ("CSeq");

/*!
 * Indicates the initiator of the request. This may be different from the initiator of the dialog. 
 * Requests sent by the callee to the caller use the callee's address in the From header field. The optional 
 * "display-name" is meant to be rendered by a human user interface. A system SHOULD use the display name "Anonymous" 
 * if the identity of the client is to remain hidden. Even if the "display-name" is empty, the "name-addr" form MUST 
 * be used if the "addr-spec" contains a comma, question mark, or semicolon. From header fields are equivalent if their 
 * URIs match, and their parameters match. Extension parameters in one header field, not present in the other are 
 * ignored for the purposes of comparison. This means that the display name and presence or absence of angle brackets 
 * do not affect matching.
 */
const std::string SIP_HEADER_FROM("From");

/*!
 * The Max-Forwards header field must be used with any SIP method to limit the number of proxies or gateways
 * that can forward the request to the next downstream server.  This can also be useful when the client is attempting
 * to trace a request chain that appears to be failing or looping in mid-chain.
 * 
 * The Max-Forwards value is an integer in the range 0-255 indicating the remaining number of times this request 
 * message is allowed to be forwarded.  This count is decremented by each server that forwards the request. 
 * The recommended initial value is 70.
 */
const std::string SIP_HEADER_MAX_FORWARDS("Max-Forwards");
const unsigned char SIP_MAX_FORWARDS = 70;

/*!
 * This field MAY appear in any request within a dialog, in any CANCEL request and in any response whose status code 
 * explicitly allows the presence of this header field.
 */
const std::string SIP_HEADER_REASON("Reason");

/*!
 * Specifies the logical recipient of the request. 
 * 
 * The optional "display-name" is meant to be rendered by a human-user interface. 
 * The "tag" parameter serves as a general mechanism for dialog identification.
 */
const std::string SIP_HEADER_TO("To");

/*!
 * The User-Agent header field contains information about the UAC originating the request.  The semantics of this 
 * header field are defined in [H14.43].
 * 
 * Revealing the specific software version of the user agent might allow the user agent to become more vulnerable to 
 * attacks against software that is known to contain security holes.  Implementers SHOULD make the User-Agent header 
 * field a configurable option.
 */
const std::string SIP_HEADER_USER_AGENT("User-Agent");

/*!
 * The Via header field indicates the path taken by the request so far and indicates the path that should be followed 
 * in routing responses. The branch ID parameter in the Via header field values serves as a transaction identifier, 
 * and is used by proxies to detect loops.
 * 
 * A Via header field value contains the transport protocol used to send the message, the client's host name or network 
 * address, and possibly the port number at which it wishes to receive responses.  A Via header field value can also 
 * contain parameters such as "maddr", "ttl", "received", and "branch", whose meaning and use are described 
 * in other sections.  For implementations compliant to this specification, the value of the branch parameter MUST 
 * start with the magic cookie "z9hG4bK", as discussed in Section 8.1.1.7.
 * 
 * Transport protocols defined here are "UDP", "TCP", "TLS", and "SCTP". "TLS" means TLS over TCP.  When a request is 
 * sent to a SIPS URI, the protocol still indicates "SIP", and the transport protocol is TLS.
 */
const std::string SIP_HEADER_VIA("Via");

struct HeaderField : public KeyValuePair<std::string>
{
    HeaderField() : KeyValuePair()
    {
    }

    HeaderField(std::string key, std::string value) : KeyValuePair(key, value)
    {
    }
};

struct SIPHeader : KeyValuePairs<HeaderField>
{
    virtual ~SIPHeader()
    {
        //required for vtable for dynamic cast
    }
    
    const int getContentLength() const
    {
        return atoi(operator[](SIP_HEADER_CONTENT_LENGTH).data());
    }
    
    virtual const std::string getRequestCommand() const = 0;
    
    /*!
     * \return the branch-tag from the Via header-field or an empty string if no such tag exists
     */
    const std::string getBranchTag() const
    {
        const std::string& viaField = operator [](SIP_HEADER_VIA);
        std::string::size_type index = viaField.find("branch=");
        if(index == std::string::npos)
        {
            return "";
        }
        index += strlen("branch=");
        return viaField.substr(index, viaField.find(';', index) - index);
    }
    
    virtual const std::string getRemoteTag() const = 0;
    
    /*!
     * This method returns the user-name, host-name IP-address and port of the originating device for this 
     * SIPHeader. The host-name may be the same as the IP-address, if the host-name was specified in numeric form
     * 
     * NOTE: for this method to work, the header needs to have a Contact or a From header-field
     * 
     * \return the remote user-name, remote host-name, remote IP-address and remote port for this request
     */
    const std::tuple<std::string, std::string, std::string, int> getAddress() const
    {
        //read contact-field
        //Contact/From: [<user-name>] "<sip:"<user>"@"<host>[":"<port>]">"
        const std::string& headerField = this->operator [](SIP_HEADER_CONTACT).empty() ? this->operator [](SIP_HEADER_FROM) : this->operator [](SIP_HEADER_CONTACT);
        std::string::size_type index1 = headerField.find('<');
        index1 += std::string("<sip:").size();
        std::string::size_type index2 = headerField.find('@', index1);
        const std::string user = headerField.substr(index1, index2 - index1);
        index2 += 1;
        index1 = headerField.find_first_of(":>", index2);
        const std::string host = headerField.substr(index2, index1 - index2);
        //host may be host-name or IP-address (but we need to skip possible URL-attributes starting with ';')
        const std::string ipAddress = Utility::getAddressForHostName(host.find(';') != std::string::npos ? host.substr(0, host.find(';')) : host);
        int port = -1;
        if(headerField[index1] == ':')
        {
            index1 += 1;
            index2 = headerField.find('>', index1);
            port = atoi(headerField.substr(index1, index2 - index1).data());
        }
        return std::make_tuple(user, host, ipAddress, port);
    }
};

struct SIPRequestHeader : public SIPHeader
{
    std::string requestCommand;
    std::string requestURI;
    
    SIPRequestHeader() : SIPHeader(), requestCommand(""), requestURI("")
    {
    }
    
    SIPRequestHeader(const std::string& requestCommand, const std::string& requestURI) : 
        SIPHeader(), requestCommand(requestCommand), requestURI(requestURI)
    {
    }
    
    const std::string getRequestCommand() const
    {
        return requestCommand;
    }
    
    const std::string getRemoteTag() const
    {
        const std::string& fromField = operator [](SIP_HEADER_FROM);
        if(fromField.find("tag=") == std::string::npos)
        {
            return "";
        }
        std::string::size_type index = fromField.find("tag=") + strlen("tag=");
        return fromField.substr(index, fromField.find(';', index) - index);
    }
};

////
// SIP Response Codes (assortment)
//  for a more complete list, see http://www.networksorcery.com/enp/protocol/sip.htm
////

//Provisional 1xx
/*
 * Provisional responses, also known as informational responses, indicate that the server contacted is performing some 
 * further action and does not yet have a definitive response
 */
//This response indicates that the request has been received by the next-hop server and that some unspecified action is being taken on behalf of this call
const unsigned int SIP_RESPONSE_TRYING_CODE = 100;
const std::string SIP_RESPONSE_TRYING("Trying");

const unsigned int SIP_RESPONSE_DIALOG_ESTABLISHED_CODE = 101;
const std::string SIP_RESPONSE_DIALOG_ESTABLISHED("Dialog Establishement");

//The UA receiving the INVITE is trying to alert the user
const unsigned int SIP_RESPONSE_RINGING_CODE= 180;
const std::string SIP_RESPONSE_RINGING("Ringing");

//A server MAY use this status code to indicate that the call is being forwarded to a different set of destinations
const unsigned int SIP_RESPONSE_CALL_FORWARDED_CODE = 181;
const std::string SIP_RESPONSE_CALL_FORWARDED("Call is being forwarded");

//The called party is temporarily unavailable, but the server has decided to queue the call rather than reject it
const unsigned int SIP_RESPONSE_QUEUED_CODE = 182;
const std::string SIP_RESPONSE_QUEUED("Queued");

//Successful 2xx
/*
 * The request was successful.
 */
//The request has succeeded
const unsigned int SIP_RESPONSE_OK_CODE = 200;
const std::string SIP_RESPONSE_OK("OK");

//Redirection 3xx
/*
 * 3xx responses give information about the user's new location, or about alternative services that might be able to satisfy the call
 */

//The address in the request resolved to several choices, each with its own specific location, 
//and the user (or UA) can select a preferred communication end point and redirect its request to that location
//The choices SHOULD also be listed as Contact fields (Section 20.10)
const unsigned int SIP_RESPONSE_MULTIPLE_CHOICES_CODE = 300;
const std::string SIP_RESPONSE_MULTIPLE_CHOICES("Multiple choices");

//The user can no longer be found at the address in the Request-URI, and the requesting client SHOULD 
//retry at the new address given by the Contact header field (Section 20.10)
const unsigned int SIP_RESPONSE_MOVED_PERMANENTLY_CODE = 301;
const std::string SIP_RESPONSE_MOVED_PERMANENTLY("Moved permanently");

//The requesting client SHOULD retry the request at the new address(es) given by the Contact header field (Section 20.10)
const unsigned int SIP_RESPONSE_MOVED_TEMPORARILY_CODE = 302;
const std::string SIP_RESPONSE_MOVED_TEMPORARILY("Moved temporarily");

//The requested resource MUST be accessed through the proxy given by the Contact field
const unsigned int SIP_RESPONSE_USE_PROXY_CODE = 305;
const std::string SIP_RESPONSE_USE_PROXY("Use proxy");

//The call was not successful, but alternative services are possible.
const unsigned int SIP_RESPONSE_ALTERNATIVE_SERVICE_CODE = 380;
const std::string SIP_RESPONSE_ALTERNATIVE_SERVICE("Alternative service");

//Request Failure 4xx
/*
 * 4xx responses are definite failure responses from a particular server
 */
//The request could not be understood due to malformed syntax.
//The Reason-Phrase SHOULD identify the syntax problem in more detail, for example, "Missing Call-ID header field".
const unsigned int SIP_RESPONSE_BAD_REQUEST_CODE = 400;
const std::string SIP_RESPONSE_BAD_REQUEST("Bad request");

//The request requires user authentication
const unsigned int SIP_RESPONSE_UNAUTHORIZED_CODE = 401;
const std::string SIP_RESPONSE_UNAUTHORIZED("Unauthorized");

//The server understood the request, but is refusing to fulfill it
const unsigned int SIP_RESPONSE_FORBIDDEN_CODE = 403;
const std::string SIP_RESPONSE_FORBIDDEN("Forbidden");

//The server has definitive information that the user does not exist at the domain specified in the Request-URI
const unsigned int SIP_RESPONSE_NOT_FOUND_CODE = 404;
const std::string SIP_RESPONSE_NOT_FOUND("Not found");

//The method specified in the Request-Line is understood, but not allowed for the address identified by the Request-URI.
const unsigned int SIP_RESPONSE_METHOD_NOT_ALLOWED_CODE = 405;
const std::string SIP_RESPONSE_METHOD_NOT_ALLOWED("Method not allowed");

//The resource identified by the request is only capable of generating response entities that have 
//content characteristics not acceptable according to the Accept header field sent in the request
const unsigned int SIP_RESPONSE_NOT_ACCEPTABLE_CODE = 406;
const std::string SIP_RESPONSE_NOT_ACCEPTABLE("Not acceptable");

//This code is similar to 401 (Unauthorized), but indicates that the client MUST first authenticate itself with the proxy
const unsigned int SIP_RESPONSE_PROXY_AUTHENTICATION_REQUIRED_CODE = 407;
const std::string SIP_RESPONSE_PROXY_AUTHENTICATION_REQUIRED("Proxy authentication required");

//The server could not produce a response within a suitable amount of time, for example, if it could not determine the location of the user in time
const unsigned int SIP_RESPONSE_REQUEST_TIMEOUT_CODE = 408;
const std::string SIP_RESPONSE_REQUEST_TIMEOUT("Request timeout");

//The requested resource is no longer available at the server and no forwarding address is known
const unsigned int SIP_RESPONSE_GONE_CODE = 410;
const std::string SIP_RESPONSE_GONE("Gone");

//The server is refusing to process a request because the request entity-body is larger than the server is willing or able to process
//If the condition is temporary, the server SHOULD include a Retry-After header field to indicate that it is temporary and after what time the client MAY try again.
const unsigned int SIP_RESPONSE_REQUEST_ENTITY_TOO_LARGE_CODE = 413;
const std::string SIP_RESPONSE_REQUEST_ENTITY_TOO_LARGE("Request entity too large");

//The server is refusing to service the request because the Request-URI is longer than the server is willing to interpret.
const unsigned int SIP_RESPONSE_REQUEST_URI_TOO_LONG_CODE = 414;
const std::string SIP_RESPONSE_REQUEST_URI_TOO_LONG("Request URI too long");

//The server is refusing to service the request because the message body of the request is in a format not supported by the server for the requested method
//The server MUST return a list of acceptable formats using the Accept, Accept-Encoding, or Accept-Language header field, depending on the specific problem with the content
const unsigned int SIP_RESPONSE_UNSUPPORTED_MEDIA_TYPE_CODE = 415;
const std::string SIP_RESPONSE_UNSUPPORTED_MEDIA_TYPE("Unsupported media type");

//The server cannot process the request because the scheme of the URI in the Request-URI is unknown to the server
const unsigned int SIP_RESPONSE_UNSUPPORTED_SCHEME_CODE = 416;
const std::string SIP_RESPONSE_UNSUPPORTED_SCHEME("Unsupported URI scheme");

//The server did not understand the protocol extension specified in a Proxy-Require (Section 20.29) or Require (Section 20.32) header field
const unsigned int SIP_RESPONSE_BAD_EXTENSION_CODE = 420;
const std::string SIP_RESPONSE_BAD_EXTENSION("Bad extension");

//The UAS needs a particular extension to process the request, but this extension is not listed in a Supported header field in the request
const unsigned int SIP_RESPONSE_EXTENSION_REQUIRED_CODE = 421;
const std::string SIP_RESPONSE_EXTENSION_REQUIRED("Extension required");

//The server is rejecting the request because the expiration time of the resource refreshed by the request is too short
const unsigned int SIP_RESPONSE_INTERVAL_TOO_BRIEF_CODE = 423;
const std::string SIP_RESPONSE_INTERVAL_TOO_BRIEF("Interval too brief");

//The callee's end system was contacted successfully but the callee is currently unavailable
//The response MAY indicate a better time to call in the Retry-After header field.
//The reason phrase SHOULD indicate a more precise cause as to why the callee is unavailable
const unsigned int SIP_RESPONSE_TEMPORARILY_UNAVAILABLE_CODE = 480;
const std::string SIP_RESPONSE_TEMPORARILY_UNAVAILABLE("Temporarily unavailable");

//This status indicates that the UAS received a request that does not match any existing dialog or transaction.
const unsigned int SIP_RESPONSE_CALL_TRANSACTION_NOT_EXIST_CODE = 481;
const std::string SIP_RESPONSE_CALL_TRANSACTION_NOT_EXIST("Call/Transaction does not exist");

//The server has detected a loop.
const unsigned int SIP_RESPONSE_LOOP_DETECTED_CODE = 482;
const std::string SIP_RESPONSE_LOOP_DETECTED("Loop detected");

//The server received a request that contains a Max-Forwards header field with the value zero
const unsigned int SIP_RESPONSE_TOO_MANY_HOPS_CODE = 483;
const std::string SIP_RESPONSE_TOO_MANY_HOPS("Too many hops");

//The server received a request with a Request-URI that was incomplete.
//Additional information SHOULD be provided in the reason phrase.
const unsigned int SIP_RESPONSE_ADDRESS_INCOMPLETE_CODE = 484;
const std::string SIP_RESPONSE_ADDRESS_INCOMPLETE("Address incomplete");

//The Request-URI was ambiguous.  The response MAY contain a listing of possible unambiguous addresses in Contact header fields
const unsigned int SIP_RESPONSE_AMBIGUOUS_CODE = 485;
const std::string SIP_RESPONSE_AMBIGUOUS("Ambiguous");

//The callee's end system was contacted successfully, but the callee is currently not willing or able to take additional calls at this end system
//The response MAY indicate a better time to call in the Retry-After header field
const unsigned int SIP_RESPONSE_BUSY_CODE = 486;
const std::string SIP_RESPONSE_BUSY("Busy here");

//The request was terminated by a BYE or CANCEL request
const unsigned int SIP_RESPONSE_REQUEST_TERMINATED_CODE = 487;
const std::string SIP_RESPONSE_REQUEST_TERMINATED("Request terminated");

//The response has the same meaning as 606 (Not Acceptable), but only applies to the specific resource addressed by 
//the Request-URI and the request may succeed elsewhere
const unsigned int SIP_RESPONSE_NOT_ACCEPTABLE_HERE_CODE = 488;
const std::string SIP_RESPONSE_NOT_ACCEPTABLE_HERE("Not acceptable here");

//The request was received by a UAS that had a pending request within the same dialog
const unsigned int SIP_RESPONSE_REQUEST_PENDING_CODE = 491;
const std::string SIP_RESPONSE_REQUEST_PENDING("Request pending");

//The request was received by a UAS that contained an encrypted MIME body for which the recipient does not possess or will not provide an appropriate decryption key
const unsigned int SIP_RESPONSE_UNDECIPHERABLE_CODE = 493;
const std::string SIP_RESPONSE_UNDECIPHERABLE("Undecipherable");

//Server Failure 5xx
/*
 * 5xx responses are failure responses given when a server itself has erred.
 */
//The server encountered an unexpected condition that prevented it from fulfilling the request
const unsigned int SIP_RESPONSE_SERVER_ERROR_CODE = 500;
const std::string SIP_RESPONSE_SERVER_ERROR("Server internal error");

//The server does not support the functionality required to fulfill the request
const unsigned int SIP_RESPONSE_NOT_IMPLEMENTED_CODE = 501;
const std::string SIP_RESPONSE_NOT_IMPLEMENTED("Not implemented");

//The server, while acting as a gateway or proxy, received an invalid response from the downstream server it accessed in attempting to fulfill the request
const unsigned int SIP_RESPONSE_BAD_GATEWAY_CODE = 502;
const std::string SIP_RESPONSE_BAD_GATEWAY("Bad gateway");

//The server is temporarily unable to process the request due to a temporary overloading or maintenance of the server
const unsigned int SIP_RESPONSE_SERVICE_UNAVIALABLE_CODE = 503;
const std::string SIP_RESPONSE_SERVICE_UNAVAILABLE("Service unavailable");

//The server did not receive a timely response from an external server it accessed in attempting to process the request
const unsigned int SIP_RESPONSE_SERVER_TIMEOUT_CODE = 504;
const std::string SIP_RESPONSE_SERVER_TIMEOUT("Server time-out");

//The server does not support, or refuses to support, the SIP protocol version that was used in the request
const unsigned int SIP_RESPONSE_VERSION_NOT_SUPPORTED_CODE = 505;
const std::string SIP_RESPONSE_VERSION_NOT_SUPPORTED("Version not supported");

//The server was unable to process the request since the message length exceeded its capabilities
const unsigned int SIP_RESPONSE_MESSAGE_TOO_LARGE_CODE = 513;
const std::string SIP_RESPONSE_MESSAGE_TOO_LARGE("Message too large");

//Global Failures 6xx
/*
 * 6xx responses indicate that a server has definitive information about a particular user, 
 * not just the particular instance indicated in the Request-URI.
 */
//The callee's end system was contacted successfully but the callee is busy and does not wish to take the call at this time.
const unsigned int SIP_RESPONSE_BUSY_EVERYWHERE_CODE = 600;
const std::string SIP_RESPONSE_BUSY_EVERYWHERE("Busy everywhere");

//The callee's machine was successfully contacted but the user explicitly does not wish to or cannot participate
const unsigned int SIP_RESPONSE_DECLINE_CODE = 603;
const std::string SIP_RESPONSE_DECLINE("Decline");

//The server has authoritative information that the user indicated in the Request-URI does not exist anywhere.
const unsigned int SIP_RESPONSE_DOES_NOT_EXIST_ANYWHERE_CODE = 604;
const std::string SIP_RESPONSE_DOES_NOT_EXIST_ANYWHERE("Does not exist anywhere");

//The user's agent was contacted successfully but some aspects of the session description such as the requested media, bandwidth, or addressing style were not acceptable.
const unsigned int SIP_RESPONSE_NOT_ACCEPTABLE_ANYWHERE_CODE = 606;
const std::string SIP_RESPONSE_NOT_ACCEPTABLE_ANYWHERE("Not acceptable");

struct SIPResponseHeader : public SIPHeader
{
    unsigned int statusCode;
    std::string reasonPhrase;
    
    SIPResponseHeader() : SIPHeader(), statusCode(0), reasonPhrase("")
    {
    }
    
    SIPResponseHeader(const unsigned int statusCode, const std::string& message) : 
        SIPHeader(), statusCode(statusCode), reasonPhrase(message)
    {
    }

    const std::string getRequestCommand() const
    {
        const std::string cSeq = operator[](SIP_HEADER_CSEQ);
        //request-command is everything after the first space ' '
        return cSeq.substr(cSeq.find(' ')+1);
    }
    
    const std::string getRemoteTag() const
    {
        const std::string& toField = operator [](SIP_HEADER_TO);
        if(toField.find("tag=") == std::string::npos)
        {
            return "";
        }
        std::string::size_type index = toField.find("tag=") + strlen("tag=");
        return toField.substr(index, toField.find(';', index) - index);
    }
};

/*!
 * Handler for the Session Initiation Protocol (SIP) specified in RFC 3261 (https://tools.ietf.org/html/rfc3261).
 * 
 * NOTE: this implementation is not thread-safe!
 * 
 * Sources for this implementation:
 * https://tools.ietf.org/html/rfc3261
 * http://www.networksorcery.com/enp/protocol/sip.htm
 * http://www.siptutorial.net/SIP/index.html
 */
class SIPPackageHandler
{
public:
    SIPPackageHandler();
    
    static inline const std::string createSIPURI(const std::string& user, const std::string& host, int port)
    {
        //sip:user@host[:port]
        return (std::string("sip:") + user + "@") + host + (port > 0 ? std::string(":") + std::to_string(port): std::string());
    }
    
    static const std::string createRequestPackage(const SIPRequestHeader& header, const std::string& requestBody);
    
    static const std::string readRequestPackage(const void* sipPackage, unsigned int packageLength, SIPRequestHeader& readHeader);
    
    static const std::string createResponsePackage(const SIPResponseHeader& header, const std::string& responseBody);
    
    static const std::string readResponsePackage(const void* sipPackage, unsigned int packageLength, SIPResponseHeader& readHeader);
    
    /*!
     * Returns whether the package in the buffer is a valid SIP response-package
     */
    static bool isResponsePackage(const void* buffer, const unsigned int bufferLength);
    
    /*!
     * Returns whether the package in the buffer is a valid SIP request-package
     */
    static bool isRequestPackage(const void* buffer, const unsigned int bufferLength);
    
    /*!
     * \param header The SIPHeader to check
     * 
     * \return whether this package has a multi-part body
     */
    static bool hasMultipartBody(const SIPHeader& header);
    
    /*!
     * Reads the multi-part body and returns the single parts mapped as (MIME-type -> part). See RFC 5621
     * 
     * \param header The SIPHeader to get the multi-part bodies
     * 
     * \param body The body-text of the whole body
     * 
     * \return a map of all occurring MIME-types and their content
     */
    static std::map<std::string, std::string> readMultipartBody(const SIPHeader& header, const std::string& body);
    
private:
    
    static void writeHeaderFields(std::stringstream& stream, const std::vector<HeaderField> fields, unsigned int contentSize);
    
    /*!
     * Request and Response are very similar in their structure, so we can unify the read-methods
     * 
     * \param sipPackage The buffer containing the package
     * 
     * \param packageLength The maximum length of the SIP package
     * 
     * \param header The vector to write the header-fields into
     * 
     * \return a vector containing the following values:
     *      For request: request-command, request-uri, SIP-version, body
     *      For response: SIP-version, response-status, response-reason, body
     */
    static std::vector<std::string> readPackage(const void* sipPackage, unsigned int packageLength, std::vector<HeaderField>& header);
};

#endif	/* SIPPACKAGEHANDLER_H */


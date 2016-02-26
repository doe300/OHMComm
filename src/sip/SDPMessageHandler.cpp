/* 
 * File:   SDPMessageHandler.cpp
 * Author: daniel
 * 
 * Created on November 26, 2015, 2:43 PM
 */

#include <iostream>

#include "sip/SDPMessageHandler.h"
#include "rtp/RTCPHeader.h"
#include "sip/SIPPackageHandler.h"
#include "network/NetworkGrammars.h"

//clang seems to need the static char-fields to be declared for linker to find them
constexpr char SessionDescription::SDP_VERSION;
constexpr char SessionDescription::SDP_ORIGIN;
constexpr char SessionDescription::SDP_SESSION_NAME;
constexpr char SessionDescription::SDP_CONNECTION;
constexpr char SessionDescription::SDP_TIMING;
constexpr char SessionDescription::SDP_MEDIA;
constexpr char SessionDescription::SDP_ATTRIBUTE;

const std::string SessionDescription::SDP_ATTRIBUTE_RTPMAP("rtpmap");
const std::string SessionDescription::SDP_ATTRIBUTE_FMTP("fmtp");
const std::string SessionDescription::SDP_ATTRIBUTE_RTCP("rtcp");
const std::string SessionDescription::SDP_MEDIA_RTP("RTP/AVP");
const std::string SessionDescription::SDP_MEDIA_SRTP("RTP/SAVP");

SDPMessageHandler::SDPMessageHandler()
{
}

std::string SDPMessageHandler::createSessionDescription(const std::string& localUserName, const NetworkConfiguration& config, const std::vector<MediaDescription>& media)
{
    NTPTimestamp now = NTPTimestamp::now();
    std::string localIP = Utility::getLocalIPAddress(Utility::getNetworkType(config.remoteIPAddress));
    std::string addrType = NetworkGrammars::isIPv6Address(localIP) ? "IP6" : "IP4";
    
    std::vector<std::string> lines;
    //required: v, o, s, t, m
    //Version
    //v=0
    lines.push_back("v=0");
    
    //Origin
    //o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    //<username> is the user's login on the originating host, or it is "-"
    //<sess-id> is a numeric string such that the tuple of <username>, <sess-id>, <nettype>, <addrtype>, and 
    //  <unicast-address> forms a globally unique identifier for the session
    //  [...] a Network Time Protocol (NTP) format timestamp be used to ensure uniqueness
    //<sess-version> is a version number for this session description [...] Again, it is RECOMMENDED that an NTP format timestamp is used
    //<nettype> is a text string giving the type of network [...] "IN" is defined to have the meaning "Internet"
    //<addrtype> is a text string giving the type of the address that follows. Initially "IP4" and "IP6" are defined
    //<unicast-address> is the address of the machine from which the session was created
    lines.push_back(std::string("o=").append(localUserName + " ").append(std::to_string(now.getSeconds()))
            .append(" ").append(std::to_string(now.getSeconds())).append(" IN ").append(addrType)
            .append(" ").append(localIP));
    
    //Session Name
    //s=<session name>
    //The "s=" field is the textual session name. [...] The "s=" field MUST NOT be empty
    lines.push_back(std::string("s=-"));
    
    //Session Information
    //i=<session description>
    //[...]provide a free-form human-readable description of the session
    //not required -> not supported
    
    //URI
    //u=<uri>
    //The URI should be a pointer to additional information about the session
    //not required -> not supported
    
    //Email Address and Phone Number
    //e=<email-address>
    //p=<phone-number>
    //not required -> not supported
    
    //Connection Data
    //c=<nettype> <addrtype> <connection-address>
    //<nettype>, <addrtype> as above
    //<connection-address> the unicast IP address of the expected data source
    lines.push_back(std::string("c=IN ").append(addrType).append(" ").append(localIP));
    
    //Bandwidth
    //b=<bwtype>:<bandwidth>
    //<bwtype> CT or AS (interpreted to be application specific)
    //<bandwidth> kilobits per second
    //XXX maybe we could estimate the bandwidth from audio-format, samplerate and codec ??
    //not required -> not supported
    
    //Timing
    //t=<start-time> <stop-time>
    //<start-time> <stop-time> start-/stop-time for session. These values are the decimal representation of Network 
    //  Time Protocol (NTP) time values in seconds since 1900
    //If the <stop-time> is set to zero, then the session is not bounded
    lines.push_back(std::string("t=").append(std::to_string(now.getSeconds()).append(" 0")));
    
    //Repeat Times
    //r=<repeat interval> <active duration> <offsets from start-time>
    //we don't repeat -> not supported
    
    //Time Zones
    //z=<adjustment time> <offset> <adjustment time> <offset> ....
    //not required -> not supported
    
    //Encryption Keys
    //k=<method> or k=<method>:<encryption key>
    //not required -> not supported
    
    //Media Descriptions
    //m=<media> <port> <proto> <fmt> ...
    //<media> is the media type.  Currently defined media are "audio", "video", "text", "application", and "message"
    //<port> is the transport port to which the media stream is sent [...] such as the RTP Control Protocol (RTCP) 
    //  port MAY be derived algorithmically
    //<proto> is the transport protocol "udp", "RTP/AVP" (RTP with Audio/Video Profile forMinimal Control over UDP), "RTP/SAVP"
    //  (SRTP over UDP)
    //<fmt> is a media format description [...] If the <proto> sub-field is "RTP/AVP" or "RTP/SAVP" the <fmt> sub-fields contain RTP payload type numbers
    //  we support Opus and PCM, so we send these payload-types
    std::string mediaLine = std::string("m=audio ").append(std::to_string(config.localPort)).append(" RTP/AVP ");
    if(media.empty())
    {
        //suggest all supported media-types
        for(const SupportedFormat& format : SupportedFormats::getFormats())
        {
            mediaLine.append(std::to_string(format.payloadType)).append(" ");
        }
    }
    else
    {
        //set only selected media
        for(const MediaDescription& format : media)
        {
            mediaLine.append(std::to_string(format.payloadType)).append(" ");
        }
    }
    lines.push_back(mediaLine);
    //XXX media for SRTP
    
    //Attributes
    //a=<attribute> or a=<attribute>:<value>
    //a=cat:<category> This attribute gives the dot-separated hierarchical category of the session
    //a=keywds:<keywords>
    //a=tool:<name and version of tool>
    lines.push_back(std::string("a=tool:OHMComm/")+OHMCOMM_VERSION);
    //a=ptime:<packet time> This gives the length of time in milliseconds represented by the media in a packet
    //a=maxptime:<maximum packet time> This gives the maximum amount of media that can be encapsulated
    //  in each packet, expressed as time in milliseconds
    //a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding parameters>] maps from an RTP payload type number
    //  to an encoding name denoting the payload format to be used [...] For audio streams, <encoding parameters> 
    //  indicates the number of audio channels
    //a=fmtp:<format> <format specific parameters>T parameters that are specific to a particular format to be conveyed 
    //  in a way that SDP does not have to understand them
    if(media.empty())
    {
        //RTPmap for all supported media-formats
        for(const SupportedFormat& format: SupportedFormats::getFormats())
        {
            //formats which are predefined in RFC 3551 could be skipped
            //but RFC 3264 recommends to not do so, to allow "easier migration away from static payload types" (page 6)
            lines.push_back(std::string("a=rtpmap:").append(std::to_string(format.payloadType)).append(" ")
                .append(format.encoding).append("/").append(std::to_string(format.sampleRate)).append("/").append(std::to_string(format.numChannels)));
            if(!format.parameterLine.empty())
            {
                lines.push_back(std::string("a=fmtp:").append(std::to_string(format.payloadType)).append(" ").append(format.parameterLine));
            }
        }
    }
    else
    {
        //RTPmap for selected media-formats
        for(const MediaDescription& format: media)
        {
            lines.push_back(std::string("a=rtpmap:").append(std::to_string(format.payloadType)).append(" ")
                    .append(format.encoding).append("/").append(std::to_string(format.sampleRate)).append("/").append(std::to_string(format.numChannels)));
            if(!format.getFormat().parameterLine.empty())
            {
                //add the parameters to the response too
                //XXX to be precise, only received (and understood) parameters are to be added
                lines.push_back(std::string("a=fmtp:").append(std::to_string(format.payloadType)).append(" ").append(format.getFormat().parameterLine));
            }
        }
    }
    //a=recvonly This specifies that the tools should be started in receive-only mode where applicable
    //a=sendrecv This specifies that the tools should be started in send and receive mode
    //a=sendonly This specifies that the tools should be started in send-only mode
    //a=inactive This specifies that the tools should be started in inactive mode
    //  If none of the attributes "sendonly", "recvonly", "inactive", and "sendrecv" is present, "sendrecv" SHOULD be 
    //  assumed as the default
    //a=orient:<orientation> Normally this is only used for a whiteboard or presentation tool
    //a=type:<conference type> This specifies the type of the conference.  Suggested values are "broadcast", 
    //  "meeting", "moderated", "test", and "H332"
    //a=charset:<character set> This specifies the character set to be used to display the session name and information data
    //a=sdplang:<language tag> specifies the language for the session description
    //a=lang:<language tag> specifies the default language for the session being described
    //a=framerate:<frame rate> This gives the maximum video frame rate in frames/sec
    //a=quality:<quality> suggestion for the quality of the encoding as an integer value
    //  10 - the best still-image quality possible
    //  5  - the default behavior given no quality suggestion.
    //  0  - the worst still-image quality the codec designer thinks is still usable.
    
    //SDES Cryptographic extension - https://tools.ietf.org/html/rfc4568
    //a=crypto:<tag> <crypto-suite> <key-params> [<session-params>]
    //<tag> is a decimal number used as an identifier for a particular crypto attribute
    //<crypto-suite> is an identifier that describes the encryption and authentication algorithms
    //<key-params> provides one or more sets of keying material for the crypto-suite in question
    //  key-params = <key-method> ":" <key-info>
    //  <key-method> is always "inline" for SRTP
    //  <key-info> is the actual keying material
    //  for SRTP: "inline:" <key||salt> ["|" lifetime] ["|" MKI ":" length]
    //<session-params> are specific to a given transport and use of them is OPTIONAL in the security descriptions framework
    //  - KDR:    The SRTP Key Derivation Rate is the rate at which a pseudo-random function is applied to a master key.
    //  - UNENCRYPTED_SRTP:      SRTP messages are not encrypted.
    //  - UNENCRYPTED_SRTCP:     SRTCP messages are not encrypted.
    //  - UNAUTHENTICATED_SRTP:  SRTP messages are not authenticated.
    //  - FEC_ORDER:   Order of forward error correction (FEC) relative to SRTP services.
    //  - FEC_KEY:     Master Key for FEC when the FEC stream is sent to a separate address and/or port.
    //  - WSH:         Window Size Hint.
    //  - Extensions:  Extension parameters can be defined.
    if(false)   //XXX if SRTP is enabled
    {
        lines.push_back(std::string("a=crypto:0 ").append(""/* XXX Algorithm*/).append(" inline:").append(""/*XXX Key*/)
                .append(" "/*XXX session-params*/));
    }
    
    std::string result("");
    for(const std::string& line: lines)
    {
        result.append(line).append(CRLF);
    }
    return result;
}

const SessionDescription SDPMessageHandler::readSessionDescription(std::string message)
{
    //Extract lines from message
    std::vector<std::string> lines;
    std::string::size_type lastIndex = 0;
    std::string::size_type index;
    while((index = message.find(CRLF, lastIndex)) != std::string::npos)
    {
        lines.push_back(message.substr(lastIndex, index - lastIndex));
        lastIndex = index + 2;
    }
    
    //Convert lines to description-fields
    SessionDescription descr;
    descr.fields.reserve(lines.size());
    for(const std::string& line: lines)
    {
        SessionKey field;
        field.fromString(line, '=');
        descr.fields.push_back(field);
    }
    return descr;
}

std::vector<MediaDescription> SDPMessageHandler::readMediaDescriptions(const SessionDescription& sdp)
{
    std::cout << "SDP: Reading media descriptions..." << std::endl;
    std::vector<MediaDescription> results;
    const std::vector<std::string> mediaFields = sdp.getFieldValues(SessionDescription::SDP_MEDIA);
    for(const std::string& mediaField : mediaFields)
    {
        //m=<media> <port> <proto> <fmt> ...
        if(mediaField.substr(0, strlen("audio")).compare("audio") != 0)
        {
            //skip non-audio
            continue;
        }
        std::string::size_type index = mediaField.find(' ') + 1;
        const int port = atoi(mediaField.substr(index, mediaField.find(' ', index)- index).data());
        index = mediaField.find(' ', index) + 1;
        const std::string protocol = mediaField.substr(index, mediaField.find(' ', index) - index);
        if(SessionDescription::SDP_MEDIA_RTP.compare(protocol) != 0 && SessionDescription::SDP_MEDIA_SRTP.compare(protocol) != 0)
        {
            //skip non-(S)RTP
            continue;
        }
        //the remainder of the line are space-separated payload-types
        while((index = mediaField.find(' ', index)) != std::string::npos)
        {
            //skip the found space
            index += 1;
            const unsigned int payloadType = atoi(mediaField.substr(index, mediaField.find(' ', index) - index).data());
            //check if payload-type is predefined
            if(payloadType <= PayloadType::G729)
            {
                if(payloadType == PayloadType::L16_2)
                {
                    results.push_back(MediaDescription(*(SupportedFormats::getFormat(PayloadType::L16_2)), (unsigned short)port, protocol));
                }
                else if(payloadType == PayloadType::PCMA)
                {
                    results.push_back(MediaDescription(*(SupportedFormats::getFormat(PayloadType::PCMA)), (unsigned short)port, protocol));
                }
                else if(payloadType == PayloadType::PCMU)
                {
                    results.push_back(MediaDescription(*(SupportedFormats::getFormat(PayloadType::PCMU)), (unsigned short)port, protocol));
                }
                else
                {
                    //we don't support any of the other predefined profiles
                    continue;
                }
            }
            else //otherwise load RTP-map for payload-type
            {
                MediaDescription descr = getRTPMap(sdp, payloadType);
                descr.port = port;
                descr.protocol = protocol;
                readFormatParameters(descr, sdp, payloadType);
                if(isEncodingSupported(descr.encoding))
                {
                    results.push_back(descr);
                }
            }
        }
    }
    std::cout << "SDP: " << results.size() << " useful media descriptions found" << std::endl;
    return std::move(results);
}

NetworkConfiguration SDPMessageHandler::readRTCPAttribute(const SessionDescription& sdp)
{
    NetworkConfiguration config{0};
    config.remoteIPAddress = "";
    //a=rtcp:<port> [<nettype> <addrtype> <connection-address>]
    const std::string rtcpAttribute = sdp.getAttribute(SessionDescription::SDP_ATTRIBUTE_RTCP);
    if(rtcpAttribute.empty())
    {
        //no custom RTCP-config set
        return config;
    }
    if(rtcpAttribute.find(' ') != std::string::npos)
    {
        //we contain spaces
        config.remotePort  = atoi(rtcpAttribute.substr(0, rtcpAttribute.find(' ')).c_str());
        //<netttype> must be "IN", <addrtype> "IP4" or "IP6", but we skip the checking for now
        std::string::size_type index = rtcpAttribute.find_last_of(' ') + 1;
        config.remoteIPAddress = rtcpAttribute.substr(index);
    }
    else
    {
        config.remotePort = atoi(rtcpAttribute.c_str());
    }
    return config;
}

void SDPMessageHandler::checkSessionDescription(const SessionDescription* sdp)
{
    if(sdp == nullptr)
    {
        throw std::invalid_argument("Can't check nullptr!");
    }
    //check version
    if(!sdp->hasKey(SessionDescription::SDP_VERSION) || atoi((sdp->operator [](SessionDescription::SDP_VERSION)).data()) != 0)
        throw std::invalid_argument("Invalid SDP version!");
    //check origin
    if(!sdp->hasKey(SessionDescription::SDP_ORIGIN))
        throw std::invalid_argument("SDP origin missing!");
    //check session
    if(!sdp->hasKey(SessionDescription::SDP_SESSION_NAME))
        throw std::invalid_argument("SDP session-name missing!");
}

MediaDescription SDPMessageHandler::getRTPMap(const SessionDescription& sdp, const unsigned int payloadType)
{
    //a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding parameters>]
    const std::string rtpMap = sdp.getAttribute(SessionDescription::SDP_ATTRIBUTE_RTPMAP, std::to_string(payloadType));
    if(rtpMap.empty())
    {
        return std::move(MediaDescription{});
    }
    std::string::size_type index = rtpMap.find(' ') + 1;
    const std::string encoding = rtpMap.substr(index, rtpMap.find('/', index) - index);
    index = rtpMap.find('/', index) +1;
    const unsigned int sampleRate = atoi(rtpMap.substr(index, rtpMap.find('/', index) - index).data());
    index = rtpMap.find('/', index);
    unsigned short numChannels = 2;
    if(index != std::string::npos)
    {
        index += 1;
        numChannels = atoi(rtpMap.substr(index).data());
    }
    return std::move(MediaDescription(0, "", payloadType, encoding, sampleRate, numChannels));
}

void SDPMessageHandler::readFormatParameters(MediaDescription& descr, const SessionDescription& sdp, const unsigned int payloadType)
{
    
    const std::string fmtParams = sdp.getAttribute(SessionDescription::SDP_ATTRIBUTE_FMTP, std::to_string(payloadType));
    if(fmtParams.empty())
    {
        return;
    }
    
    //the format parameters depend on the format used
    if(descr.encoding.compare("opus") == 0)
    {
        //a=fmtp:<payload type> key=value;key1=value1;...
        std::string::size_type index = fmtParams.find(' ');
        while(index != std::string::npos && index < fmtParams.size())
        {
            const std::string pair = fmtParams.substr(index + 1, fmtParams.find(';', index+1) - (index+1));
            const std::string key = Utility::trim(pair.substr(0, pair.find('=')));
            const std::string value = Utility::trim(pair.substr(pair.find('=')+1));
            descr.formatParams[key] = value;
            index = fmtParams.find(';', index + 1);
        }
    }
}


bool SDPMessageHandler::isEncodingSupported(const std::string& encoding)
{
    for(const SupportedFormat& format : SupportedFormats::getFormats())
    {
        if(Utility::equalsIgnoreCase(format.encoding, encoding))
        {
            return true;
        }
    }
    return false;
}

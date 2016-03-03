/* 
 * File:   SDPMessageHandler.h
 * Author: daniel
 *
 * Created on November 26, 2015, 2:43 PM
 */

#ifndef SDPMESSAGEHANDLER_H
#define	SDPMESSAGEHANDLER_H

#include <stdlib.h>
#include <exception>

#include "configuration.h"
#include "KeyValuePairs.h"
#include "PayloadType.h"
#include "sip/SupportedFormats.h"

struct SessionKey : public KeyValuePair<char>
{
    SessionKey() : KeyValuePair<char>()
    {
    }

    SessionKey(char key, std::string value) : KeyValuePair<char>(key, value)
    {
    }
};

struct FormatParameter : public KeyValuePair<std::string>
{
    FormatParameter() : KeyValuePair<std::string>()
    {
    }

    FormatParameter(std::string key, std::string value) : KeyValuePair<std::string>(key, value)
    {
    }
};

struct MediaDescription
{
    unsigned short port;
    std::string protocol;
    unsigned int payloadType;
    std::string encoding;
    unsigned int sampleRate;
    unsigned short numChannels;
    KeyValuePairs<FormatParameter> formatParams;
    
    MediaDescription() = default;
    
    MediaDescription(unsigned short port, const std::string& protocol, unsigned int payloadType, const std::string& encoding, unsigned int sampleRate, unsigned short numChannels) : 
        port(port), protocol(protocol), payloadType(payloadType), encoding(encoding), sampleRate(sampleRate), numChannels(numChannels)
    {
        
    }
    
    MediaDescription(const SupportedFormat& format, unsigned short port, const std::string& protocol) : port(port), protocol(protocol), 
        payloadType(format.payloadType), encoding(format.encoding), sampleRate(format.sampleRate), numChannels(format.numChannels)
    {
    }
    
    /*!
     * \return the underlying supported-format for this media-description
     */
    const SupportedFormat getFormat() const
    {
        for(const SupportedFormat& format : SupportedFormats::getFormats())
        {
            if(Utility::equalsIgnoreCase(format.encoding, encoding) && format.sampleRate == sampleRate && format.numChannels == numChannels)
            {
                return format;
            }
        }
        return SupportedFormat(0, "", 0, 0, "");
    }
};

struct SessionDescription : public KeyValuePairs<SessionKey>
{
    static constexpr char SDP_VERSION='v';
    static constexpr char SDP_ORIGIN='c';
    static constexpr char SDP_SESSION_NAME='s';
    static constexpr char SDP_CONNECTION='c';
    static constexpr char SDP_TIMING='t';
    static constexpr char SDP_MEDIA='m';
    static constexpr char SDP_ATTRIBUTE='a';
    
    static const std::string SDP_ATTRIBUTE_RTPMAP;
    static const std::string SDP_ATTRIBUTE_FMTP;
    //specifies RTCP-port, if not consecutive to RTP-port, see RFC 3605
    static const std::string SDP_ATTRIBUTE_RTCP;

    static const std::string SDP_MEDIA_RTP;
    static const std::string SDP_MEDIA_SRTP;
    
    /*!
     * \param tag The attribute-tag to search for
     * \param firstValue The (optional) first part of the attribute-value
     * 
     * \return The first matching attribute-value or an empty string
     */
    const std::string getAttribute(const std::string& tag, const std::string firstValue = "") const
    {
        const std::string search = firstValue.empty() ? tag : (tag + ':').append(firstValue);
        std::vector<std::string> allAttributes = getFieldValues(SDP_ATTRIBUTE);
        for(const std::string& a : allAttributes)
        {
            if(a.find(search) == 0)
            {
                return a.substr(a.find(':') + 1);
            }
        }
        return "";
    }

    /*!
     * \return The IP address (IPv4 or IPv6) of the media source
     */
    const std::string getConnectionAddress() const
    {
        const std::string connection = this->operator [](SDP_CONNECTION);
        //c=<nettype> <addrtype> <connection-address>
        const std::string::size_type index = connection.find_last_of(' ');
        return connection.substr(index+1);
    }
};

/*!
 * Handler for the Session Description Protocol (SDP) specified in RFC 4566 (https://tools.ietf.org/html/rfc4566).
 * 
 * NOTE: this implementation is not thread-safe!
 * 
 * Sources for this implementation:
 * https://tools.ietf.org/html/rfc4566
 */
class SDPMessageHandler
{
public:
    
    SDPMessageHandler();

    /*!
     * Creates and returns the session description for the given configuration
     * 
     * \param localUserName The user-name of the local user
     * 
     * \param config The network-configuration for the RTP stream
     * 
     * \param media An optional list of media-descriptions to use, defaults to the list of supported formats
     * 
     * \return the generated SDP session-description
     */
    static std::string createSessionDescription(const std::string& localUserName, const NetworkConfiguration& config, const std::vector<MediaDescription>& media = {});
    
    /*!
     * Ready the session description from the given message and returns it
     */
    static const SessionDescription readSessionDescription(std::string message);
    
    /*!
     * This method extracts all media-descriptions from the given session-description which are valid for the OHMComm application
     * 
     * \param sdp The session-description
     * 
     * \return a list of all extracted media-descriptions
     */
    static std::vector<MediaDescription> readMediaDescriptions(const SessionDescription& sdp);
    
    /*!
     * This method will return 0 (zero) for the remote-port or an empty string for the remote-address, if they are not
     * specified in the RTCP-attribute. See RFC 3605
     * 
     * \param sdp The session-description
     * 
     * \return The custom RTCP configuration, if any
     */
    static NetworkConfiguration readRTCPAttribute(const SessionDescription& sdp);
    
    /*!
     * Checks the correctness of the given SessionDescription.
     * NOTE: This function throws std::invalid_argument exceptions with a informative messages in case of an error
     * 
     * \param sdp The SessionDescription to check
     */
    static void checkSessionDescription(const SessionDescription* sdp);
    
private:
    
    /*!
     * Reads a single media-description from a "rtpmap"-attribute
     */
    static MediaDescription getRTPMap(const SessionDescription& sdp, const unsigned int payloadType);
    
    /*!
     * Reads additional format-parameters (if any) into the given media-description
     */
    static void readFormatParameters(MediaDescription& descr, const SessionDescription& sdp, const unsigned int payloadType);
    
    /*!
     * \return whether the given encoding is supported
     */
    static bool isEncodingSupported(const std::string& encoding);
};

#endif	/* SDPMESSAGEHANDLER_H */


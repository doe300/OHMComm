/*
 * File:   RTCPPackageHandler.h
 * Author: daniel
 *
 * Created on June 16, 2015, 5:47 PM
 */

#ifndef RTCPPACKAGEHANDLER_H
#define	RTCPPACKAGEHANDLER_H

#include <string.h> // memcpy
#include <stdint.h> //uint8_t for Windows
#include <vector>
#include <string>

/*!
 * RTCP package type
 */
typedef uint8_t RTCPPackageType;

static constexpr RTCPPackageType RTCP_PACKAGE_SENDER_REPORT = 200;
static constexpr RTCPPackageType RTCP_PACKAGE_RECEIVER_REPORT = 201;
static constexpr RTCPPackageType RTCP_PACKAGE_SOURCE_DESCRIPTION = 202;
static constexpr RTCPPackageType RTCP_PACKAGE_GOODBYE = 203;
static constexpr RTCPPackageType RTCP_PACKAGE_APPLICATION_DEFINED = 204;

/*!
 * RTCP Source description (SDES) payload type
 */
typedef uint8_t RTCPSourceDescriptionType;

static constexpr RTCPSourceDescriptionType RTCP_SOURCE_CNAME = 1;
static constexpr RTCPSourceDescriptionType RTCP_SOURCE_NAME = 2;
static constexpr RTCPSourceDescriptionType RTCP_SOURCE_EMAIL = 3;
static constexpr RTCPSourceDescriptionType RTCP_SOURCE_PHONE = 4;
static constexpr RTCPSourceDescriptionType RTCP_SOURCE_LOC = 5;
static constexpr RTCPSourceDescriptionType RTCP_SOURCE_TOOL = 6;
static constexpr RTCPSourceDescriptionType RTCP_SOURCE_NOTE = 7;

static constexpr uint8_t RTCP_HEADER_SIZE = 8;
static constexpr uint8_t RTCP_SENDER_INFO_SIZE = 20;
static constexpr uint8_t RTCP_RECEPTION_REPORT_SIZE = 24;

/*!
 * The RTCP header has the following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|  RC/SC  |      PT       |             length            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         SSRC of sender                        |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * The header, is 8 octets long. The fields have the following meaning:
 *
 * version (V): 2 bits
 *  Identifies the version of RTP, which is the same in RTCP packets as in RTP data packets.
 *  The version defined by this specification is two (2).
 *
 * padding (P): 1 bit
 *  If the padding bit is set, this RTCP packet contains some additional padding octets at the end
 *  which are not part of the control information. The last octet of the padding is a count of how many padding octets
 *  should be ignored. Padding may be needed by some encryption algorithms with fixed block sizes.
 *  In a compound RTCP packet, padding should only be required on the last individual packet
 *  because the compound packet is encrypted as a whole.
 *
 * reception report count (RC): 5 bits
 *  The number of reception report blocks contained in this SR/RR packet. A value of zero is valid.
 *
 * source count (SC): 5 bits
 *  The number of SSRC/CSRC chunks contained in this SDES packet. A value of zero is valid but useless.
 *
 * packet type (PT): 8 bits
 *  Contains the RTCPPackageType identifying the package-type.
 *
 * length: 16 bits
 *  The length of this RTCP packet in 32-bit words minus one, including the header and any padding.
 *  (The offset of one makes zero a valid length and avoids a possible infinite loop in scanning a compound RTCP packet,
 *  while counting 32-bit words avoids a validity check for a multiple of 4.)
 *
 * SSRC: 32 bits
 *  The synchronization source identifier for the originator of this SR packet.
 */
struct RTCPHeader
{
    //2 bit version field
    unsigned int version : 2;

    //1 bit padding flag
    unsigned int padding : 1;

    //5 bit reception report count (RC) or source count (SC) field
    unsigned int receptionReportOrSourceCount : 5;

    //8 bit package-type field
    RTCPPackageType packageType;

    //16 bit length field
    unsigned int length : 16;

    //32 bit ssrc field
    unsigned int ssrc : 32;

    /*!
     * Creates a new RTCPHeader
     *
     * \param ssrc The SSRC, the only parameter not written by the create*Package-methods
     */
    RTCPHeader(uint32_t ssrc) : ssrc(ssrc)
    {
        //version is always 2 per specification
        version = 2;
        //padding is 0 by default
        padding = 0;
        //RC/SC is zero by default
        receptionReportOrSourceCount = 0;
    }

};

/*!
 * The Sender Information has the following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |              NTP timestamp, most significant word             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             NTP timestamp, least significant word             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         RTP timestamp                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     sender's packet count                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      sender's octet count                     |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * The sender information, is 20 octets long and is present in every sender report packet.
 * It summarizes the data transmissions from this sender. The fields have the following meaning:
 *
 * NTP timestamp: 64 bits
 *  Indicates the wallclock time when this report was sent so that it may be used in combination with timestamps
 *  returned in reception reports from other receivers to measure round-trip propagation to those receivers.
 *  Receivers should expect that the measurement accuracy of the timestamp may be limited to far less than the resolution
 *  of the NTP timestamp. The measurement uncertainty of the timestamp is not indicated as it may not be known.
 *  A sender that can keep track of elapsed time but has no notion of wallclock time may use the elapsed time
 *  since joining the session instead. This is assumed to be less than 68 years, so the high bit will be zero.
 *  It is permissible to use the sampling clock to estimate elapsed wallclock time.
 *  A sender that has no notion of wallclock or elapsed time may set the NTP timestamp to zero.
 *
 * RTP timestamp: 32 bits
 *  Corresponds to the same time as the NTP timestamp (above), but in the same units and with the same random offset
 *  as the RTP timestamps in data packets. This correspondence may be used for intra- and inter-media synchronization
 *  for sources whose NTP timestamps are synchronized, and may be used by media- independent receivers to estimate
 *  the nominal RTP clock frequency. Note that in most cases this timestamp will not be equal to the RTP timestamp in
 *  any adjacent data packet. Rather, it is calculated from the corresponding NTP timestamp using the relationship
 *  between the RTP timestamp counter and real time as maintained by periodically checking the wallclock time
 *  at a sampling instant.
 *  NOTE: in our implementation, the RT(C)P timestamp is always in milliseconds precision
 *
 * sender's packet count: 32 bits
 *  The total number of RTP data packets transmitted by the sender since starting transmission up until the time
 *  this SR packet was generated. The count is reset if the sender changes its SSRC identifier.
 *
 * sender's octet count: 32 bits
 *  The total number of payload octets (i.e., not including header or padding) transmitted in RTP data packets
 *  by the sender since starting transmission up until the time this SR packet was generated.
 *  The count is reset if the sender changes its SSRC identifier.
 *  This field can be used to estimate the average payload data rate.
 */
struct SenderInformation
{
    //64 bit NTP timestamp field
    uint64_t NTPTimestamp: 64;

    //32 bit RTP timestamp field
    unsigned int RTPTimestamp : 32;

    //32 bit sender's package count field
    unsigned int packetCount : 32;

    //32 bit sender's octet count field
    unsigned int octetCount : 32;

    SenderInformation(uint32_t rtpTimestamp, uint32_t packageCount, uint32_t octetCount) :
    RTPTimestamp(rtpTimestamp), packetCount(packageCount), octetCount(octetCount)
    {
        //default NTP timestamp
        NTPTimestamp = 0;
    }
};

/*!
 * The Reception Report has the following format:
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |                 SSRC_1 (SSRC of first source)                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | fraction lost |       cumulative number of packets lost       |
 * -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           extended highest sequence number received           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      interarrival jitter                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         last SR (LSR)                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   delay since last SR (DLSR)                  |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |                 SSRC_2 (SSRC of second source)                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * :                               ...                             :
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |                  profile-specific extensions                  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * This section contains zero or more reception report blocks depending on the number of other sources heard
 * by this sender since the last report. Each reception report block conveys statistics on the reception
 * of RTP packets from a single synchronization source. Receivers do not carry over statistics when a source changes
 * its SSRC identifier due to a collision. These statistics are:
 *
 * SSRC_n (source identifier): 32 bits
 *  The SSRC identifier of the source to which the information in this reception report block pertains.
 *
 * fraction lost: 8 bits
 *  The fraction of RTP data packets from source SSRC_n lost since the previous SR or RR packet was sent,
 *  expressed as a fixed point number with the binary point at the left edge of the field.
 *  (That is equivalent to taking the integer part after multiplying the loss fraction by 256.)
 *  This fraction is defined to be the number of packets lost divided by the number of packets expected,
 *  as defined in the next paragraph. An implementation is shown in Appendix A.3.
 *  If the loss is negative due to duplicates, the fraction lost is set to zero.
 *  Note that a receiver cannot tell whether any packets were lost after the last one received,
 *  and that there will be no reception report block issued for a source if all packets from that source sent
 *  during the last reporting interval have been lost.
 *
 * cumulative number of packets lost: 24 bits
 *  The total number of RTP data packets from source SSRC_n that have been lost since the beginning of reception.
 *  This number is defined to be the number of packets expected less the number of packets actually received,
 *  where the number of packets received includes any which are late or duplicates.
 *  Thus packets that arrive late are not counted as lost, and the loss may be negative if there are duplicates.
 *  The number of packets expected is defined to be the extended last sequence number received, as defined next,
 *  less the initial sequence number received. This may be calculated as shown in Appendix A.3.
 *
 * extended highest sequence number received: 32 bits
 *  The low 16 bits contain the highest sequence number received in an RTP data packet from source SSRC_n,
 *  and the most significant 16 bits extend that sequence number with the corresponding count of sequence number cycles,
 *  which may be maintained according to the algorithm in Appendix A.1. Note that different receivers within the
 *  same session will generate different extensions to the sequence number if their start times differ significantly.
 *
 * interarrival jitter: 32 bits
 *  An estimate of the statistical variance of the RTP data packet interarrival time, measured in timestamp units
 *  and expressed as an unsigned integer. The interarrival jitter J is defined to be the mean deviation
 *  (smoothed absolute value) of the difference D in packet spacing at the receiver compared to the sender for a pair
 *  of packets. As shown in the equation below, this is equivalent to the difference in the "relative transit time"
 *  for the two packets; the relative transit time is the difference between a packet's RTP timestamp and the
 *  receiver's clock at the time of arrival, measured in the same units.
 *
 * last SR timestamp (LSR): 32 bits
 *  The middle 32 bits out of 64 in the NTP timestamp (as explained in Section 4) received as part of the most recent
 *  RTCP sender report (SR) packet from source SSRC_n. If no SR has been received yet, the field is set to zero.
 *
 * delay since last SR (DLSR): 32 bits
 *  The delay, expressed in units of 1/65536 seconds, between receiving the last SR packet from source SSRC_n and
 *  sending this reception report block. If no SR packet has been received yet from SSRC_n, the DLSR field is set to zero.
 *
 */
struct ReceptionReport
{
    //32 bit SSRC field
    unsigned int ssrc : 32;

    //8 bit fraction lost field
    unsigned int fraction_lost : 8;

    //24 bit cumulative number of packages lost field
    unsigned int cumulativePackageLoss : 24;

    //32 bit extended highest sequence number received field
    unsigned int extendedHighestSequenceNumberReceived : 32;

    //32 bit interarrival jitter field
    unsigned int interarrivalJitter : 32;

    //32 bit last SR timestamp field
    unsigned int lastSRTimestamp : 32;

    //32 bit delay since last SR field
    unsigned int delaySinceLastSR : 32;
};

/*!
 * The Source Description (SDES) has following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     TYPE      |     length    | user and domain name         ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * NOTE: Currently only source-descriptions for a single source are supported.
 */
struct SourceDescription
{
    //8 bit type field
    RTCPSourceDescriptionType type;

    //variable length value
    std::string value;
};

/*!
 * The application defined (APP) has following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          name (ASCII)                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   application-dependent data                 ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * The APP packet is intended for experimental use as new applications and new features are developed,
 * without requiring packet type value registration. APP packets with unrecognized names should be ignored.
 * After testing and if wider use is justified, it is recommended that each APP packet be redefined without
 * the subtype and name fields and registered with the Internet Assigned Numbers Authority using an RTCP packet type.
 *
 * version (V), padding (P), length:
 *  As described for the SR packet (see Section 6.3.1).
 *
 * subtype: 5 bits
 *  May be used as a subtype to allow a set of APP packets to be defined under one unique name,
 *  or for any application-dependent data.
 *
 * packet type (PT): 8 bits
 *  Contains the constant 204 to identify this as an RTCP APP packet.
 *
 * name: 4 octets
 *  A name chosen by the person defining the set of APP packets to be unique with respect to other APP packets
 *  this application might receive. The application creator might choose to use the application name, and then
 *  coordinate the allocation of subtype values to others who want to define new packet types for the application.
 *  Alternatively, it is recommended that others choose a name based on the entity they represent, then coordinate
 *  the use of the name within that entity. The name is interpreted as a sequence of four ASCII characters,
 *  with uppercase and lowercase characters treated as distinct.
 *
 * application-dependent data: variable length
 *  Application-dependent data may or may not appear in an APP packet.
 *  It is interpreted by the application and not RTP itself. It must be a multiple of 32 bits long.
 */
struct ApplicationDefined
{
    //4 byte name field
    char name[4];

    //length for application specific data
    uint16_t dataLength;

    //variable length application specific data
    char *data;

    //5 bit application defined sub-type -> is stored in RTCPHeader.receptionReportOrSourceCount
    unsigned int subType : 5;

    ApplicationDefined(const char name[4], uint16_t dataLength, char *data, uint8_t subType) : dataLength(dataLength), data(data), subType(subType)
    {
        memcpy(this->name, name, 4);
    }
};

/*!
 * Utility-class to create/read RTCP packages
 *
 * NOTE: This class is not thread safe!
 *
 * NOTE: All methods in this class modify some of their parameter!
 */
class RTCPPackageHandler
{
public:

    RTCPPackageHandler();

    virtual ~RTCPPackageHandler();

    /*!
     * Creates a new sender report (SR) package
     *
     * \param header The package-header
     *
     * \param senderInfo The SenderInformation
     *
     * \param reports A (possible empty) list of reception reports
     *
     * \return A pointer to the created package
     */
    void *createSenderReportPackage(RTCPHeader &header, SenderInformation &senderInfo, std::vector<ReceptionReport> reports);

    /*!
     * Creates a new receiver report (RR) package
     *
     * \param header The package-header
     *
     * \param reports A (possible empty) list of reception reports
     *
     * \return A pointer to the created package
     */
    void *createReceiverReportPackage(RTCPHeader &header, std::vector<ReceptionReport> reports);

    /*!
     * Creates a new source description (DES) package
     *
     * \param header The header of this package
     *
     * \param descriptions The vector of SourceDescription
     *
     * \return A pointer to the created package
     */
    void *createSourceDescriptionPackage(RTCPHeader &header, std::vector<SourceDescription> descriptions);

    /*!
     * Creates a new BYE package
     *
     * \param header The header
     *
     * \param byeMessage The message to send with this package (possible empty)
     *
     * \return A pointer to the created package
     */
    void *createByePackage(RTCPHeader &header, std::string byeMessage);

    /*!
     * Creates a new APP package
     *
     * \param header The header
     *
     * \param appData The application specific data
     *
     * \return A pointer to the created package
     */
    void *createApplicationDefinedPackage(RTCPHeader &header, ApplicationDefined &appDefined);

    /*!
     * Reads a sender report (SR) package
     *
     * \param senderReportPackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \param senderInfo The SenderInformation to store the read sender-info into
     *
     * \return a list of read reception-reports, may be empty
     */
    std::vector<ReceptionReport> readSenderReport(void *senderReportPackage, uint16_t packageLength, RTCPHeader &header, SenderInformation &senderInfo);

    /*!
     * Reads a receiver report (RR) package
     *
     * \param receiverReportPackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return a list of read reception-reports, may be empty
     */
    std::vector<ReceptionReport> readReceiverReport(void *receiverReportPackage, uint16_t packageLength, RTCPHeader &header);

    /*!
     * Reads a source description (SDES) package
     *
     * \param sourceDescriptionPackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return the read descriptions
     */
    std::vector<SourceDescription> readSourceDescription(void *sourceDescriptionPackage, uint16_t packageLength, RTCPHeader &header);

    /*!
     * Reads a BYE package
     *
     * \param byePackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return the bye-message attached to the package, may be empty
     */
    std::string readByeMessage(void *byePackage, uint16_t packageLength, RTCPHeader &header);

    /*!
     * Reads a APP package
     *
     * \param appDefinedPackage The buffer to read from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return the read ApplicationDefined data
     */
    ApplicationDefined readApplicationDefinedMessage(void *appDefinedPackage, uint16_t packageLength, RTCPHeader &header);

    /*!
     * Reads an RTCP-header and returns whether the package was an RTCP-package
     *
     * \param rtcpPackage The buffer to read from
     *
     * \param packageLength The number of bytes in the package
     *
     * \return Whether the RTCP-header was successfully read
     */
    RTCPHeader readRTCPHeader(void* rtcpPackage, unsigned int packageLength);

    /*!
     * This method tries to determine whether the received buffer holds an RTCP package.
     *
     * NOTE: Though this method can distinguish RTCP-packages from RTP-packages,
     * there may be some type of packages, which are not distinguished and falsely accepted as valid RTCP-package.
     *
     * \param packageBuffer The buffer storing the package-data
     *
     * \param packageLength The length of the package in bytes
     *
     * \return Whether this buffer COULD be holding hold an RTCP package
     */
    bool isRTCPPackage(void* packageBuffer, unsigned int packageLength );

    /*!
     * \param lengthHeaderField The value of the RTCP header-field "length"
     *
     * \return the length of the RTCP-package in bytes
     */
    unsigned int getRTCPPackageLength(unsigned int lengthHeaderField);

private:
    char *rtcpPackageBuffer;

    /*!
     * Calculates the length-field from the given length in bytes
     *
     * Specification: "The length of this RTCP packet in 32-bit words minus one, including the header and any padding"
     */
    uint8_t calculateLengthField(uint16_t length);
};

#endif	/* RTCPPACKAGEHANDLER_H */


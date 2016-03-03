/* 
 * File:   RTPHeader.h
 * Author: daniel
 *
 * Created on January 22, 2016, 10:12 AM
 */

#ifndef RTPHEADER_H
#define	RTPHEADER_H

#include <stdexcept>
//For htons/htonl and ntohs/ntohl
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "PayloadType.h"

/*!
 * A RTP header extension has the following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      defined by profile       |           length              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        header extension                       |
 * |                             ....                              |
 *
 * If the X bit in the RTP header is one, a variable-length header extension MUST be appended to the RTP header,
 * following the CSRC list if present.
 * The header extension contains a 16-bit length field that counts the number of 32-bit words in the extension,
 * excluding the four-octet extension header (therefore zero is a valid length). Only a single extension
 * can be appended to the RTP data header. To allow multiple interoperating implementations to each experiment
 * independently with different header extensions, or to allow a particular implementation to experiment
 * with more than one type of header extension, the first 16 bits of the header extension are left open
 * for distinguishing identifiers or parameters. The format of these 16 bits is to be defined
 * by the profile specification under which the implementations are operating.
 *
 * This RTP specification does not define any header extensions itself.
 */
struct RTPHeaderExtension
{
private:
    //16 bit defined by profile
    unsigned int profile_field : 16;

    //16 bit length field
    unsigned int length : 16;

    //list of 32 bit header extensions
    uint32_t* extensions;
    
public:
    /*!
    * Minimum size for RTP header-extensions, 4 Byte
    */
    static constexpr unsigned int MIN_EXTENSION_SIZE{4};
    
    RTPHeaderExtension(uint16_t length) : profile_field(0), length(length)
    {
        if(length > 0)
        {
            extensions = new uint32_t[length];
        }
        else
        {
            extensions = nullptr;
        }
    }
    
    ~RTPHeaderExtension()
    {
        delete[] extensions;
    }
    
    inline uint16_t getProfile() const
    {
        return ntohs(profile_field);
    }
    
    inline void setProfile(uint16_t profile) 
    {
        profile_field = htons(profile);
    }
    
    inline uint16_t getLength() const
    {
        return ntohs(length);
    }
    
    inline uint32_t* getExtension()
    {
        return extensions;
    }
    
    inline const uint32_t* getExtension() const
    {
        return extensions;
    }
};

/*!
 *
 * The RTP header is specified in RFC 3550 has the following format:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           timestamp                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           synchronization source (SSRC) identifier            |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |            contributing source (CSRC) identifiers             |
 * |                             ....                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * The first twelve octets are present in every RTP packet, while the
 * list of CSRC identifiers is present only when inserted by a mixer.
 * The fields have the following meaning:
 *
 * version (V): 2 bits
 *  This field identifies the version of RTP. The version defined by this specification is two (2).
 *
 * padding (P): 1 bit
 *  If the padding bit is set, the packet contains one or more additional padding octets at the end
 *  which are not part of the payload. The last octet of the padding contains a count of how many padding octets
 *  should be ignored, including itself. Padding may be needed by some encryption algorithms with fixed block sizes
 *  or for carrying several RTP packets in a lower-layer protocol data unit.
 *
 * extension (X): 1 bit
 *  If the extension bit is set, the fixed header MUST be followed by exactly one header extension,
 *  with a format defined in Section 5.3.1.
 *
 * CSRC count (CC): 4 bits
 *  The CSRC count contains the number of CSRC identifiers that follow the fixed header.
 *
 * marker (M): 1 bit
 *  The interpretation of the marker is defined by a profile. It is intended to allow significant events
 *  such as frame boundaries to be marked in the packet stream. A profile MAY define additional marker bits
 *  or specify that there is no marker bit by changing the number of bits in the payload type field (see Section 5.3).
 *
 * payload type (PT): 7 bits
 *  This field identifies the format of the RTP payload and determines its interpretation by the application.
 *  A profile MAY specify a default static mapping of payload type codes to payload formats.
 *  Additional payload type codes MAY be defined dynamically through non-RTP means (see Section 3).
 *  A set of default mappings for audio and video is specified in the companion RFC 3551 [1].
 *  An RTP source MAY change the payload type during a session, but this field SHOULD NOT be used
 *  for multiplexing separate media streams (see Section 5.2).
 *  A receiver MUST ignore packets with payload types that it does not understand.
 *
 * sequence number: 16 bits
 *  The sequence number increments by one for each RTP data packet sent, and may be used by the receiver
 *  to detect packet loss and to restore packet sequence. The initial value of the sequence number SHOULD be random
 *  (unpredictable) to make known-plaintext attacks on encryption more difficult, even if the source itself does not
 *  encrypt according to the method in Section 9.1, because the packets may flow through a translator that does.
 *
 * timestamp: 32 bits
 *  The timestamp reflects the sampling instant of the first octet in the RTP data packet.
 *  The sampling instant MUST be derived from a clock that increments monotonically and linearly in time to allow
 *  synchronization and jitter calculations (see Section 6.4.1). The resolution of the clock MUST be sufficient
 *  for the desired synchronization accuracy and for measuring packet arrival jitter
 *  (one tick per video frame is typically not sufficient). The clock frequency is dependent on the format of data
 *  carried as payload and is specified statically in the profile or payload format specification
 *  that defines the format, or MAY be specified dynamically for payload formats defined through non-RTP means.
 *  If RTP packets are generated periodically, the nominal sampling instant as determined from the sampling clock
 *  is to be used, not a reading of the system clock.
 *  As an example, for fixed-rate audio the timestamp clock would likely increment by one for each sampling period.
 *  If an audio application reads blocks covering 160 sampling periods from the input device,
 *  the timestamp would be increased by 160 for each such block, regardless of whether the block is transmitted
 *  in a packet or dropped as silent.
 *  The initial value of the timestamp SHOULD be random, as for the sequence number.
 *  Several consecutive RTP packets will have equal timestamps if they are (logically) generated at once,
 *  e.g., belong to the same video frame.  Consecutive RTP packets MAY contain timestamps that are not monotonic
 *  if the data is not transmitted in the order it was sampled, as in the case of MPEG interpolated video frames.
 *  RTP timestamps from different media streams may advance at different rates and usually have independent,
 *  random offsets.
 *  The sampling instant is chosen as the point of reference for the RTP timestamp because it is known
 *  to the transmitting endpoint and has a common definition for all media,
 *  independent of encoding delays or other processing.
 *  Applications transmitting stored data rather than data sampled in real time typically use a virtual presentation
 *  timeline derived from wallclock time to determine when the next frame or other unit of each medium
 *  in the stored data should be presented.
 *  NOTE: in our implementation, the RT(C)P timestamp is always in milliseconds precision
 *
 * SSRC: 32 bits
 *  The SSRC field identifies the synchronization source. This identifier SHOULD be chosen randomly,
 *  with the intent that no two synchronization sources within the same RTP session will have the same SSRC identifier.
 *  An example algorithm for generating a random identifier is presented in Appendix A.6.
 *  Although the probability of multiple sources choosing the same identifier is low,
 *  all RTP implementations must be prepared to detect and resolve collisions. Section 8 describes the probability of
 *  collision along with a mechanism for resolving collisions and detecting RTP-level forwarding loops
 *  based on the uniqueness of the SSRC identifier. If a source changes its source transport address,
 *  it must also choose a new SSRC identifier to avoid being interpreted as a looped source (see Section 8.2).
 *
 * CSRC list: 0 to 15 items, 32 bits each
 *  The CSRC list identifies the contributing sources for the payload contained in this packet.
 *  The number of identifiers is given by the CC field. If there are more than 15 contributing sources,
 *  only 15 can be identified. CSRC identifiers are inserted by mixers (see Section 7.1), using the SSRC identifiers of
 *  contributing sources. For example, for audio packets the SSRC identifiers of all sources that were mixed together
 *  to create a packet are listed, allowing correct talker indication at the receiver.
 */
struct RTPHeader
{
private:
    
    //data[0]
    //2 bit version field
    //1 bit padding flag
    //1 bit extension flag
    //4 bit CSRC count field
    
    //data[1]
    //1 bit marker flag
    //7 bit payload type field
    
    //data[2-3]
    //16 bit sequence number field
    
    //data[4-7]
    //32 bit timestamp field
    
    //data[8-11]
    //32 bit SSRC field

    //data[12 - 71]
    //list of 32 bit CSRCs
    //FIXME setting this to 72 creates schlieren in the audio-output
    //but exactly the same number of total/payload-bytes is sent/received for 12 and 72 bytes header??!?
    uint8_t data[12];
    
    static constexpr unsigned int shiftVersion = 6;
    static constexpr unsigned int shiftPadding = 5;
    static constexpr unsigned int shiftExtension = 4;
    static constexpr unsigned int shiftMarker = 7;
    
public:
    
    /*!
     * Minimum size of a RTP-Header in bytes, without any CSRCs set
     */
    static constexpr unsigned int MIN_HEADER_SIZE{12};
    
    /*!
     * Maximum size of a RTP-Header with all CSRCs set.
     */
    static constexpr unsigned int MAX_HEADER_SIZE{72};
    
    /*!
     * The version-flag of a RTP package is always 2
     */
    static constexpr unsigned int VERSION{2};
    
    RTPHeader() : data{0}
    {
        data[0] = VERSION << shiftVersion;
    }
    
    inline uint8_t getVersion() const
    {
        return (data[0] >> shiftVersion) & 0x3;
    }
    
    inline bool isPadded() const
    {
        return (data[0] >> shiftPadding) & 0x1;
    }
    
    inline void setPadding(bool padded)
    {
        data[0] = data[0] | (padded << shiftPadding);
    }
    
    inline bool hasExtension() const
    {
        return (data[0] >> shiftExtension) & 0x1;
    }
    
    inline void setExtension(bool extension)
    {
        data[0] = data[0] | (extension << shiftExtension);
    }
    
    inline uint8_t getCSRCCount() const
    {
        return data[0] & 0xF;
    }
    
    inline void setCSRCCount(uint8_t csrcCount)
    {
        data[0] = data[0] | (csrcCount & 0xF);
    }
    
    inline bool isMarked() const
    {
        return (data[1] >> shiftMarker) & 0x1;
    }
    
    inline void setMarker(bool marker)
    {
        data[1] = data[1] | (marker << shiftMarker);
    }
    
    inline PayloadType getPayloadType() const
    {
        return (PayloadType)(data[1] & 0x7F);
    }
    
    inline void setPayloadType(PayloadType type)
    {
        data[1] = data[1] | (type & 0x7F);
    }
        
    inline uint16_t getSequenceNumber() const
    {
        return ntohs((data[3] << 8) | data[2]);
    }
    
    inline void setSequenceNumber(const uint16_t sequenceNumber)
    {
        data[3] = (uint8_t) (htons(sequenceNumber) >> 8);
        data[2] = (uint8_t) (htons(sequenceNumber) & 0xFF);
    }
    
    inline uint32_t getTimestamp() const
    {
        return ntohl(data[7] << 24 | data[6] << 16 | data[5] << 8 | data[4]);
    }
    
    inline void setTimestamp(const uint32_t timstamp)
    {
        uint32_t tmp = htonl(timstamp);
        data[7] = (uint8_t) (tmp >> 24);
        data[6] = (uint8_t) (tmp >> 16);
        data[5] = (uint8_t) (tmp >> 8);
        data[4] = (uint8_t) (tmp & 0xFF);
    }
    
    inline uint32_t getSSRC() const
    {
        return ntohl(data[11] << 24 | data[10] << 16 | data[9] << 8 | data[8]);
    }
    
    inline void setSSRC(const uint32_t ssrc)
    {
        uint32_t tmp = htonl(ssrc);
        data[11] = (uint8_t) (tmp >> 24);
        data[10] = (uint8_t) (tmp >> 16);
        data[9] = (uint8_t) (tmp >> 8);
        data[8] = (uint8_t) (tmp & 0xFF);
    }
    
    inline uint32_t getCSRC(uint8_t index) const
    {
        if(index >= getCSRCCount())
            throw std::out_of_range("CSRC index greater than the CSRC count!");
        //offset of 12 bytes + 4 bytes per CSRC
        const uint8_t startIndex = 12 + (index << 2);
        return ntohl(data[startIndex + 3] << 24 | data[startIndex + 2] << 16 | data[startIndex + 1] << 8 | data[startIndex]);
    }
    
    inline void setCSRC(const uint8_t index, const uint32_t csrc)
    {
        if(index == getCSRCCount())
            //increase CSRC-count by one
            setCSRCCount(index + 1);
        else if(index > getCSRCCount())
            //otherwise we would convert a gap into a valid CSRC
            throw std::out_of_range("CSRCs must be added sequential!");
        //offset of 12 bytes + 4 bytes per CSRC
        const uint8_t startIndex = 12 + (index << 2);
        const uint32_t tmp = htonl(csrc);
        data[startIndex + 3] = (uint8_t) (tmp >> 24);
        data[startIndex + 2] = (uint8_t) (tmp >> 16);
        data[startIndex + 1] = (uint8_t) (tmp >> 8);
        data[startIndex] = (uint8_t) (tmp & 0xFF);
    }
};

#endif	/* RTPHEADER_H */


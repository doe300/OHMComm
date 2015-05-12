/* 
 * File:   RTPPackage.h
 * Author: daniel
 *
 * Created on March 28, 2015, 12:30 PM
 */

#ifndef RTPPACKAGE_H
#define	RTPPACKAGE_H

#include <stdint.h>
#include <stdlib.h>
//TODO byte-order (network-order)??

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
 * independently with different header extensions, or to allow aparticular implementation to experiment
 * with more than one type of header extension, the first 16 bits of the header extension are left open
 * for distinguishing identifiers or parameters. The format of these 16 bits is to be defined
 * by the profile specification under which the implementations are operating. 
 * 
 * This RTP specification does not define any header extensions itself.
 */
struct RTPHeaderExtension
{
    //16 bit defined by profile
    unsigned int profile_field: 16;
    
    //16 bit length field
    unsigned int length: 16;
    
    //list of 32 bit header extensions
    uint32_t *extensions[];
    
    /*!
     * Copies the data of this HeaderExtension to the front of the buffer.
     * 
     * \param buffer The buffer to copy into
     * 
     * \param maxBufferSize The maximum size of the buffer to fill
     * 
     * Returns the number of bytes copied or -1 if an error occurred
     */
    uint16_t copyToBuffer(char *buffer, size_t maxBufferSize);
    
    /*!
     * Reads the data of this HeaderExtension from the buffer.
     * 
     * \param buffer The buffer to read from
     * 
     * \param maxBufferSize The maximum size of the input-buffer
     * 
     * Returns the number of bytes read or -1 if an error occurred
     */
    uint16_t readFromBuffer(char *buffer, size_t maxBufferSize);
};

/*!
 * 
 * The RTP header has the following format:
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
    //2 bit version field
    unsigned int version: 2;
    
    //1 bit padding flag
    unsigned int padding: 1;
    
    //1 bit extension flag
    unsigned int extension: 1;
    
    //4 bit CSRC count field
    unsigned int csrc_count: 4;
    
    //1 bit marker flag
    unsigned int marker: 1;
    
    //7 bit payload type field
    unsigned int payload_type: 7;
    
    //16 bit sequence number field
    unsigned int sequence_number: 16;
    
    //32 bit timestamp field
    unsigned int timestamp: 32;
    
    //32 bit SSRC field
    unsigned int ssrc: 32;

    //list of 32 bit CSRCs
    uint32_t csrc_list[15];
    
    RTPHeaderExtension *header_extension;
    
    RTPHeader();
    
    /*!
     * Copies the data of this header to the front of the buffer.
     * 
     * \param buffer The buffer to copy into
     * 
     * \param maxBufferSize The maximum size of the buffer to fill
     * 
     * Returns the number of bytes copied or -1 if an error occurred
     */
    uint16_t copyToBuffer(char *buffer, size_t maxBufferSize);
    
    /*!
     * Reads the data of this header from the buffer.
     * 
     * \param buffer The buffer to read from
     * 
     * \param maxBufferSize The maximum size of the input-buffer
     * 
     * Returns the number of bytes read or -1 if an error occurred
     */
    uint16_t readFromBuffer(char *buffer, size_t maxBufferSize);
};

/*!
 * The maximum size of a RPT-header in bytes
 * TODO: Currently does not include extension!!
 */
const uint8_t RTP_HEADER_MAX_SIZE = 72;

/*!
 * List of default mappings for payload-type, as specified in https://www.ietf.org/rfc/rfc3551.txt
 * 
 * Also see: https://en.wikipedia.org/wiki/RTP_audio_video_profile
 * 
 * Currently only containing audio mappings.
 */
enum PayloadType
{
    //ITU-T G.711 PCMU - https://en.wikipedia.org/wiki/PCMU
    PCMU = 0,
    //GSM Full Rate - https://en.wikipedia.org/wiki/Full_Rate
    GSM = 3,
    //ITU-T G.723.1 - https://en.wikipedia.org/wiki/G.723.1
    G723 = 4,
    //IMA ADPCM 32 kbit/s - https://en.wikipedia.org/wiki/Adaptive_differential_pulse-code_modulation
    DVI4_32 = 5,
    //IMA ADPCM 64 kbit/s
    DVI4_64 = 6,
    //LPC - https://en.wikipedia.org/wiki/Linear_predictive_coding
    LPC = 7,
    //ITU-T G.711 PCMA - https://en.wikipedia.org/wiki/PCMA
    PCMA = 8,
    //ITU-T G.722 - https://en.wikipedia.org/wiki/G.722
    G722 = 9,
    //Linear PCM, 2 channels - https://en.wikipedia.org/wiki/Linear_PCM
    L16_2 = 10,
    //Linear PCM, 1 channel - https://en.wikipedia.org/wiki/Linear_PCM
    L16_1 = 11,
    //Qualcomm Code Excited Linear Prediction
    QCELP = 12,
    //Comfort noise
    CN = 13,
    //MPEG-1 or MPEG-2 audio - https://en.wikipedia.org/wiki/MPEG-1 / https://en.wikipedia.org/wiki/MPEG-2
    MPA = 14,
    //ITU-T G.728
    G728 = 15,
    //IMA ADPCM 44.1 kbit/s
    DVI4_44 = 16,
    //IMA ADPCM 88.2 kbit/s
    DVI4_88 = 17,
    //ITU-T G.729(a) - https://en.wikipedia.org/wiki/G.729
    G729 = 18,
    
};

struct RTPPackage
{
    //the header information
    RTPHeader header;
    //the package body
    void *package;
    //the size of the package
    size_t packageSize;
    
    RTPPackage();
    
    RTPPackage(RTPHeader header, size_t packageSize, void *packageData);
    
    /*!
     * Copies the data of this RTPPackage to the front of the buffer.
     * 
     * \param buffer The buffer to copy into
     * 
     * \param maxBufferSize The maximum size of the buffer to fill
     * 
     * Returns the number of bytes copied or -1 if an error occurred
     */
    uint16_t copyToBuffer(char *buffer, size_t maxBufferSize);
    
    /*!
     * Reads the data of this RTPPackage from the buffer.
     * 
     * \param buffer The buffer to read from
     * 
     * \param maxBufferSize The maximum size of the input-buffer
     * 
     * Returns the number of bytes read or -1 if an error occurred
     */
    uint16_t readFromBuffer(char *buffer, size_t maxBufferSize);
};

#endif	/* RTPPACKAGE_H */


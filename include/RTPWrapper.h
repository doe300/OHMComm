/* 
 * File:   RTPWrapper.h
 * Author: daniel
 *
 * Created on March 7, 2015, 1:10 PM
 */

#ifndef RTPWRAPPER_H
#define	RTPWRAPPER_H

#include <stdint.h>
#include <array>

/*
 * Some definitions (extracted from RFC 3550):
 * 
 * RTP payload: the contents of an RTP packet
 * 
 * RTP packet: a data packet with RTP header and the RTP payload
 * 
 * Transport address: combination of network address and port identifying a transport-layer endpoint
 * 
 * RTP media type: collection of payload types which can be carried within a single RTP session
 * 
 * Multimedia session: set of concurrent RTP sessions among a common group of participants
 * 
 * RTP session: An association among a set of participants communicating via RTP.
 *  I.e. a stream (audio or video), while the Multimedia session would consist of both streams
 * 
 * Synchronization source (SSRC): The source of a stream of RTP packets,
 *  identified by a 32-bit numeric SSRC carried in the RTP headers to be independent from the network address.
 *  All packets from a synchronization source form part of the same timing and sequence number space,
 *  so a receiver groups packets by synchronization source for playback.
 *  The SSRC identifier is a randomly chosen value meant to be globally unique within a particular RTP session.
 * 
 * Contributing source (CSRC): A source of a stream of RTP packets that has contributed to the combined stream
 *  produced by an RTP mixer.
 * 
 * End system: An application that generates the content to be sent in RTP packets and/or consumes the content
 *  of received RTP packets.
 * 
 * Mixer: An intermediate system that receives RTP packets from one or more sources, possibly changes the data format,
 *  combines the packets in some manner and then forwards a new RTP packet.
 *  Since the timing among multiple input sources will not generally be synchronized,
 *  the mixer will make timing adjustments among the streams and generate its own timing for the combined stream.
 *  Thus, all data packets originating from a mixer will be identified as having the mixer as their synchronization source.
 * 
 * Translator: An intermediate system that forwards RTP packets with their synchronization source identifier intact.
 * 
 * Monitor: An application that receives RTCP packets sent by participants in an RTP session,
 *  in particular the reception reports, and estimates the current quality of service for distribution monitoring,
 *  fault diagnosis and long-term statistics.
 * 
 * Byte order:
 * All integer fields are carried in network byte order, that is, most significant byte (octet) first.
 * This byte order is commonly known as big-endian.
 * 
 * Wallclock time (absolute date and time) is represented using the timestamp format of the Network Time Protocol (NTP),
 *  which is in seconds relative to 0h UTC on 1 January 1900.
 * 
 * https://www.ietf.org/rfc/rfc3550.txt
 */

//needs to be declared before being used by RTPWrapper
struct RTPHeader;

/*!
 * Provides methods to send/receive a RTP package.
 */
class RTPWrapper
{
public:
    /*!
     * Initializes the RTPWrapper.
     * Chooses a random 16bit sequence-number and a random 32bit timestamp.
     * \param socket the descriptor of the underlying socket
     * \param payloadType the application-defined payload-type
     * \param tickInterval the tick interval in milliseconds
     */
    RTPWrapper(int socket, uint8_t payloadType, int16_t tickInterval);
    RTPWrapper(const RTPWrapper& orig);
    virtual ~RTPWrapper();
    
    /*!
     * The signature of this method matches the RtAudioCallback from RTAudio.h.
     * So this method can be used directly as parameter for RTAudio#openStream
     * 
     * For further documentation of the parameters, see RtAudioCallback from RTAudio.h
     * 
     * \param outputBuffer Will be NULL for input-only streams.
     *      Otherwise the RTPWrapper should read \c nFrames of data from the connection and write into this buffer
     * 
     * \param inputBuffer Will be NULL for output-only-streams.
     *      Holds \c nFrames of input audio sample frames to be sent over RTP
     * 
     * \param nFrames The number of sample frames of input or output
     *      data in the buffers
     * 
     * \param streamTime The number of seconds that have elapsed since the
     *      stream was started. Can be used as timestamp header-field
     * 
     * \param status If non-zero, indicates data overflow or underflow-status of this stream.
     *      Overflow means that some input data was discarded by the driver.
     *      Underflow occurs when the input buffer runs low, resulting in a pause in playback.
     *      See RTAudioStreamStatus in RTAudio.h
     * 
     * \param userData A pointer to the user-data specified at stream startup, defaults to NULL
     * 
     * To continue playback, this method must return a value of zero.
     * To stop the stream and drain output-buffer, it returns one.
     * To abort immediately, two is returned.
     */
    int inout( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double streamTime, unsigned int status, void *data );
    
    //TODO error codes
    /*!
     * \param buffer is the send-buffer
     * \param length the number of bytes to send
     * \return the number of bytes sent
     */
    //int send(uint8_t *buffer, uint16_t length);
    
    /*!
     * \param buffer the receive buffer
     * \param maxTimeout the maximum timeout to wait before returning (in milliseconds)
     * \param RTPHeader the variable to write the received header-information
     * \return an status-code
     */
    //int receive(uint8_t *buffer, uint16_t maxTimeout, RTPHeader header);
    
    /*!
     * \return the sequence number for the last packet (a 16 bit integer) sent.
     */
    uint16_t getSequenceNumber();
    
    /*!
     * \return the random number of this synchronization source
     */
    uint32_t getSynchronizationSource();
    
    /*!
     * \return the number of registered contributing sources
     */
    uint8_t getContributionSourcesCount();
    
    /*!
     * \param the ID of the contribution source
     * \return -1 on error, 0 otherwise
     */
    int8_t addContributionSource(uint32_t csrc);
    
private:
    
    uint8_t contributionSourcesCount;
    uint32_t synchronizationSource;
    uint16_t currentSequenceNumber;
    std::array<uint32_t, 15> contributionSources;

    /*!
     * generates a random sequence number as initial value
     */
    uint16_t generateSequenceNumber();
    
    /*!
     * generates a random initial timestamp
     */
    uint32_t generateTimestamp();
    
    /*!
     * generates a random SSRC number
     */
    uint32_t generateSynchronizationSource();
    
    /*!
     * increments the sequence, taking overflow into account
     */
    uint16_t incrementSequenceNumber();
};
#endif	/* RTPWRAPPER_H */


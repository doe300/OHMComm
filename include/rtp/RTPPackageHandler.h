/*
 * File:   RTPPackage.h
 * Author: daniel
 *
 * Created on March 28, 2015, 12:30 PM
 */

#ifndef RTPPACKAGEHANDLER_H
#define	RTPPACKAGEHANDLER_H

#include <random> // random generator objects
#include <chrono> // clock, tick
#include <string.h> //memcpy

#include "rtp/RTPHeader.h"

class RTPPackageHandler
{
public:
    /*!
     * Constructs a new RTPPackageHandler-object
     *
     * \param maximumPayloadSize The maximum size in bytes of the payload (body)
     *
     */
    RTPPackageHandler(unsigned int maximumPayloadSize);

    virtual ~RTPPackageHandler();
    
    /*!
     * Generates a new RTP-package by generating the header and copying the payload-data (audio data)
     *
     * \param audioData The payload for the new RTP-package
     *
     * \param payloadSize The size in bytes of the audio-payload,
     *  must be smaller or equals to #getMaximumPayloadSize()
     *
     * Returns a pointer to the internal buffer storing the new package
     */
    virtual const void* createNewRTPPackage(const void* audioData, unsigned int payloadSize);

    /*!
     * Returns a pointer to the payload of the internal RTP-package
     */
    virtual const void* getRTPPackageData() const;

    /*!
     * Returns a RTPHeader pointer of the currently stored RTP-package.
     */
    virtual const RTPHeader* getRTPPackageHeader() const;
    
    /*!
     * Returns a RTPHeaderExtension pointer to the extension in the stored RTP-package. 
     * If no such extension exists, nullptr is returned.
     */
    virtual const RTPHeaderExtension getRTPHeaderExtension() const;

    /*!
     * Gets the maximum size for the RTP package (header + body)
     */
    unsigned int getMaximumPackageSize() const;

    /*!
     * Gets the RTPHeader size
     */
    unsigned int getRTPHeaderSize() const;
    
    /*!
     * Returns the size (in bytes) of the currently stored RTP header-extension
     */
    unsigned int getRTPHeaderExtensionSize() const;

    /*!
     * Gets the maximum payload size
     */
    unsigned int getMaximumPayloadSize() const;

    /*!
     * Returns the actual size of the currently buffered audio-payload
     *
     * Note: This value is only accurate if the #setActualPayloadSize() was set for the current payload
     */
    unsigned int getActualPayloadSize() const;

    /*!
     * Sets the payload-size in bytes of the currently buffered package
     *
     * \param payloadSize The new size in bytes
     */
    void setActualPayloadSize(unsigned int payloadSize);

    /*!
     * Returns the internal buffer, which could be used as receive buffer
     */
    void* getWorkBuffer();

    /*!
     * Creates a silence-package in the internal work-buffer.
     *
     * A silence-package is a RTP-package with a dummy header and zeroed out payload resulting in silence on playback.
     */
    void createSilencePackage();

    /*!
     * Returns the current RTP timestamp for the internal clock
     */
    uint32_t getCurrentRTPTimestamp() const;
    
    /*!
     * This method tries to determine whether the received buffer holds an RTP package.
     *
     * NOTE: There may be some type of packages, which are not distinguished and falsely accepted as valid RTP-package.
     *
     * \param packageBuffer The buffer storing the package-data
     *
     * \param packageLength The length of the package in bytes
     *
     * \return Whether this buffer COULD be holding hold an RTP package
     */
    static bool isRTPPackage(const void* packageBuffer, unsigned int packageLength );

protected:
    // A buffer that can store a whole RTP-Package
    void *workBuffer;

    unsigned int getRandomNumber();
    unsigned int createStartingTimestamp();

    std::mt19937 randomGenerator;

    unsigned int sequenceNr;
    unsigned int initialTimestamp;

    unsigned int currentBufferSize;
    unsigned int maximumPayloadSize;
    unsigned int maximumBufferSize;
    unsigned int actualPayloadSize;
    
    friend class ProcessorRTP;
};

#endif	/* RTPPACKAGEHANDLER_H */


/* 
 * File:   RTPBuffer.h
 * Author: daniel
 *
 * Created on March 28, 2015, 12:27 PM
 */

#ifndef RTPBUFFER_H
#define	RTPBUFFER_H

#include "RTPPackage.h"

typedef uint8_t RTPBufferStatus;
static const RTPBufferStatus RTP_BUFFER_ALL_OKAY = 0;
/*!
 * New package was not buffered, because of an overflow in the buffer
 */
static const RTPBufferStatus RTP_BUFFER_INPUT_OVERFLOW = 0x1;
/*!
 * No package to read, because of an underflow in the buffer
 */
static const RTPBufferStatus RTP_BUFFER_OUTPUT_UNDERFLOW = 0x2;

/*!
 * Serves as jitter-buffer for RTP packages
 */
class RTPBuffer
{
public:
    /*!
     * \param maxCapacity The maximum number of packages to buffer
     * \param maxDelay The maximum delay in milliseconds before dropping packages
     */
    RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay);
    RTPBuffer(const RTPBuffer& orig);
    virtual ~RTPBuffer();
    
    /*!
     * Adds a new package to the buffer
     * \param The package to add, must be allocated on the HEAP
     * 
     * Returns zero on success or one of the RTPBufferStatus-codes listed in RTPBuffer.h
     */
    RTPBufferStatus addPackage(const RTPPackage &package);
    
    /*!
     * Reads the oldest package in the buffer and writes it into the package-variable
     * \param A placeholder for the package to read, must be allocated on the HEAP
     * 
     * Note: Space can be allocated with: RTPPackage *package = (RTPPackage *)malloc(sizeof(RTPPackage));
     * 
     * Returns zero on success or one if the RTPBufferStatus-codes listed in RTPBuffer.h
     */
    RTPBufferStatus readPackage(RTPPackage &package);
    
    /*!
     * Returns the size of the buffer, the number of stored elements
     */
    uint16_t getSize();
private:
    /*!
     * The ring-buffer containing the packages
     */
    RTPPackage *ringBuffer;
    /*!
     * The maximum entries in the buffer, size of the array
     */
    uint16_t capacity;
    /*!
     * The maximum delay (in milliseconds) before dropping a package
     */
    uint16_t maxDelay;
    /*!
     * The index to write the next package to
     */
    uint16_t writeIndex;
    /*!
     * The index to read the next package from, the last position in the buffer
     */
    uint16_t readIndex;
    
    /*!
     * The number of buffered elements
     */
    uint16_t size;

    /*!
     * Increments the index in the ring
     */
    uint16_t incrementIndex(uint16_t index);
};

#endif	/* RTPBUFFER_H */


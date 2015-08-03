/* 
 * File:   RTPBuffer.h
 * Author: daniel
 *
 * Created on March 28, 2015, 12:27 PM
 */

#ifndef RTPBUFFER_H
#define	RTPBUFFER_H

#include "RTPPackageHandler.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    //mutex
#else
#include <mutex>    //std::mutex
#endif

#include <memory> //for std::unique_ptr<RTPBuffer>

/*!
 * This status is returned by the addPackage/readPackage-method to determine whether the operation did succeed
 */
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
     * \param minBufferPackages The minimum of packages to buffer before returning valid audio-data
     */
    RTPBuffer(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages = 1);
    ~RTPBuffer();

    /*!
     * Adds a new package to the buffer
     * 
     * \param package The package to add, must be allocated on the HEAP
     * 
     * \param contentSize The size in bytes of the package-content
     * 
     * Returns zero on success or one of the RTPBufferStatus-codes listed in RTPBuffer.h
     */
    RTPBufferStatus addPackage(RTPPackageHandler &package, unsigned int contentSize);

    /*!
     * Reads the oldest package in the buffer and writes it into the package-variable
     * \param A placeholder for the package to read, must be allocated on the HEAP
     * 
     * Returns zero on success or one if the RTPBufferStatus-codes listed in RTPBuffer.h
     */
    RTPBufferStatus readPackage(RTPPackageHandler &package);

    /*!
     * Returns the size of the buffer, the number of stored elements
     */
    uint16_t getSize();
private:

    /*!
     * Mutex guarding all access to ringBuffer, nextReadIndex, size and minSequenceNumber
     */
#ifdef _WIN32
    HANDLE bufferMutex;
#else
    std::mutex bufferMutex;
#endif

    /*!
     * Internal data structure to buffer RTP packages
     */
    struct RTPBufferPackage
    {
        /*!
         * The valid-state of the buffer entry
         */
        bool isValid;
        /*!
         * The RTPHeader
         */
        RTPHeader header;
        /*!
         * The package size in bytes (size of the content)
         */
        unsigned int contentSize;
        /*!
         * The size of the buffer, this is at least contentSize
         */
        unsigned int bufferSize;
        /*!
         * The package data
         */
        void *packageContent = nullptr;
        
        ~RTPBufferPackage()
        {
            if(packageContent != nullptr)
            {
                free(packageContent);
            }
        }
    };

    /*!
     * The ring-buffer containing the packages
     */
    RTPBuffer::RTPBufferPackage *ringBuffer;
    /*!
     * The maximum entries in the buffer, size of the array
     */
    const uint16_t capacity;
    /*!
     * The maximum delay (in milliseconds) before dropping a package
     */
    const uint16_t maxDelay;
    /*!
     * The minimum number of packages this buffer must contain before packages are read.
     * Until this lower limit is reached, silence-packages are returned.
     */
    const uint16_t minBufferPackages;
    /*!
     * The index to read the next package from, the last position in the buffer
     */
    uint16_t nextReadIndex;

    /*!
     * The number of buffered elements
     */
    uint16_t size;

    /*!
     * Increments the index in the ring
     */
    uint16_t incrementIndex(uint16_t index);

    /*!
     * The minimum sequence number still in buffer.
     * This should be the last sequence-number read from the buffer +1
     */
    uint16_t minSequenceNumber;

    /*!
     * Calculates the new index in the buffer
     */
    uint16_t calculateIndex(uint16_t index, uint16_t offset);

    void lockMutex();

    void unlockMutex();
};

#endif	/* RTPBUFFER_H */


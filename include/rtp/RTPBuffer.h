/*
 * File:   RTPBuffer.h
 * Author: daniel
 *
 * Created on March 28, 2015, 12:27 PM
 */

#ifndef RTPBUFFER_H
#define	RTPBUFFER_H

#include "RTPBufferHandler.h"
#include "PlayoutPointAdaption.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    //mutex
#else
#include <mutex>    //std::mutex
#endif


/*!
 * Serves as jitter-buffer for RTP packages
 */
class RTPBuffer : public RTPBufferHandler, private PlayoutPointAdaption
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
    RTPBufferStatus addPackage(const RTPPackageHandler &package, unsigned int contentSize);

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
    unsigned int getSize() const;
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
         * The timestamp (in milliseconds) this package was received
         */
        unsigned int receptionTimestamp;
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
        void *packageContent;

        RTPBufferPackage() : isValid(false), header(), receptionTimestamp(0), contentSize(0), bufferSize(0), packageContent(nullptr)
        {

        }

        ~RTPBufferPackage()
        {
            free(packageContent);
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


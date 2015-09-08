#ifndef RTPBUFFERALTERNATIVE_H
#define	RTPBUFFERALTERNATIVE_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>		//mutex
#else
#include <mutex>			//std::mutex
#endif

#include "RTPBufferHandler.h"
#include <math.h>           // log2
#include <algorithm>
#include <limits.h>

#define COUNT_OF_RTP_SEQ_NR 64 // TODO: Another magic number


/*!
 * Internal data structure to buffer/handle RTP packages in the class RTPBufferAlternative
 */
class RTPBufferPackage
{
public:
	/*!
	 * Constructs a RTPBufferPackage
	 *
	 * \param contentSize The payload size of package
	 * \param headerSize The size of the RTP-Header
	 */
	RTPBufferPackage(unsigned int contentSize, unsigned int headerSize)
	{
		this->contentSize = contentSize;
		this->headerSize = headerSize;

		this->packageContent = new char[contentSize];
		this->rtpHeader = new char[headerSize];
		this->hasBeenRead = false;
		this->hasBeenInitialized = false;
	}

	/*!
	 * Destructor makes sure allocated spaces gets free
	 */
	~RTPBufferPackage()
	{
		delete[] packageContent;
	}

	/*!
	 * Returns a pointer to the saved RTPHeader
	 */
	RTPHeader* getHeader()
	{
		return (RTPHeader*) rtpHeader;
	}

	/*!
	 * Saves a whole RTPPackage. Header- and package-size has been set in the constructor before.
	 * When a new package is added while the old package was never read by the getPacketContent()-Function
	 * the flag "hasOverflow" will be set. This indicate the information loss. The old unread package gets
	 * overwritten.
	 *
	 * \param header A pointer to a RTPHeader
	 * \param packageContent A pointer to the payloadContent (AudioData)
	 */
	void addPacket(RTPHeader *header, void *packageContent)
	{
		// Adding a new packet while the old one was not read? -> set the overflow flag
		if (hasBeenInitialized && hasBeenRead == false)
			hasOverflow = true;
		else
			hasOverflow = false;

		memcpy(this->rtpHeader, header, headerSize);
		memcpy(this->packageContent, packageContent, contentSize);
		this->creationTimestamp = 0; //TODO: get correct timestamp

		this->hasBeenRead = false;
		this->hasBeenInitialized = true;
	}

	/*!
	 * Returns a pointer to the payload (audio) data. Package will be marked as read.
	 * If a read package will be read again the underflow flag will be set. In this case
	 * a silence package will be written into the payload (audio) buffer.
	 */
	void *getPacketContent()
	{
		// Reading a packet which has already been read? -> set the underflow flag
		if (hasBeenRead == true)
			hasUnderflow = true;
		else
			hasUnderflow = false;

		hasBeenRead = true;
		return packageContent;
	}

	/*!
	 * Returns the size of the payload (audio) data
	 */
	unsigned int getPacketContentSize()
	{
		return contentSize;
	}

	/*!
	 * Returns the size of the RTPHeader
	 */
	unsigned int getHeaderSize()
	{
		return headerSize;
	}

	/*!
	 * Returns the overflow status
	 */
	bool overflow() { return hasOverflow; }

	/*!
	 * Returns the underflow status
	 */
	bool underflow() { return hasUnderflow; }

	/*!
	* Returns the underflow status
	*/
	bool hasBeenReadAlready() { return hasBeenRead; }
	/*!
	 * Indicates wheater the instance contains any readable package data
	 */
	bool isInitialized() { return hasBeenInitialized; }
private:
	void *rtpHeader = nullptr;
	void *packageContent = nullptr;
	unsigned int contentSize;
	unsigned int headerSize;

	bool hasBeenInitialized;
	bool hasBeenRead;
	bool hasOverflow;
	bool hasUnderflow;
	unsigned int creationTimestamp;
};

/*!
 * A buffer for RTP packages
 */
class RTPBufferAlternative : public RTPBufferHandler
{
public:
	/*!
	 * \param maxCapacity The maximum number of packages to buffer
	 * \param maxDelay The maximum delay in milliseconds before dropping packages
	 * \param minBufferPackages The minimum of packages to buffer before returning valid audio-data
	 */
	RTPBufferAlternative(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages);

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
	 * Reads the current package in the buffer and writes it into the package-variable
	 * \param A placeholder for the package to read, must be allocated on the HEAP
	 *
	 * Returns zero on success or one if the RTPBufferStatus-codes listed in RTPBuffer.h
	 */
	RTPBufferStatus readPackage(RTPPackageHandler &package);

	unsigned int getSize() const;
private:
	int isPowerOfTwo(unsigned int x);
	void initializeRingBuffer(unsigned int contentSize, unsigned int rtpHeaderSize);
	RTPBufferPackage **ringBuffer;
	bool copyNextPossiblePackageIntoPackage(RTPPackageHandler &package);
	bool copyNextPackageIntoPackage(RTPPackageHandler &package);
	void copySilencePackageIntoPackage(RTPPackageHandler &package);
	void incrementReadPos();
	void setCurrentReadPos();
	void lockMutex();
	void unlockMutex();

	bool isCurrentReadPosSet = false;
	uint16_t currentReadPos = 0;
	uint16_t lastReadSeqNr = 0;
	uint16_t amountOfUnderflowsInRow = 0;
	uint16_t amountOfPackages;
#ifdef _WIN32
	HANDLE bufferMutex;
#else
	std::mutex bufferMutex;
#endif
	bool bufferContainsPackages = false;
	uint16_t maxCapacity;
	uint16_t maxDelay;
	uint16_t minBufferPackages;
	uint16_t log;
};

#endif

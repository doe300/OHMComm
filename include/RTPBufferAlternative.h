#ifndef RTPBUFFER2_H
#define	RTPBUFFER2_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>		//mutex
#else
#include <mutex>			//std::mutex
#endif

#include "RTPPackageHandler.h"
#include <memory>			//for std::unique_ptr<RTPBuffer>
#include <math.h>           // log2

/*!
* This status is returned by the addPackage/readPackage-method to determine whether the operation did succeed
*/
typedef uint8_t RTPBufferStatus;
static const RTPBufferStatus RTP_BUFFER_ALL_OKAY = 0;
static const RTPBufferStatus RTP_BUFFER_INPUT_OVERFLOW = 0x1;
static const RTPBufferStatus RTP_BUFFER_OUTPUT_UNDERFLOW = 0x2;

class RTPBufferPackage
{
public:
	RTPBufferPackage(unsigned int contentSize, unsigned int headerSize)
	{
		this->contentSize = contentSize;
		this->headerSize = headerSize;

		this->packageContent = new char[contentSize];
		this->rtpHeader = new char[headerSize];
		this->hasBeenRead = false;
		this->hasBeenInitialized = false;
	}

	~RTPBufferPackage()
	{
		delete[] packageContent;
	}

	RTPHeader* getHeader()
	{
		return (RTPHeader*) rtpHeader;
	}


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
	
	void *getPacketContent()
	{
		// Reading a packet which has already been read? -> set the underflow flag
		if (hasBeenRead == true) {
			hasUnderflow = true;
			// Remove the data from it (Silence package)
			memset(this->packageContent, 0, contentSize);
		}
		else
			hasUnderflow = false;

		hasBeenRead = true;
		return packageContent;
	}

	unsigned int getPacketContentSize()
	{
		return contentSize;
	}

	unsigned int getHeaderSize()
	{
		return headerSize;
	}

	bool overflow() { return hasOverflow; }
	bool underflow() { return hasUnderflow; }
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

class RTPBufferAlternative
{
public:
	RTPBufferAlternative(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages);

	RTPBufferStatus addPackage(RTPPackageHandler &package, unsigned int contentSize);
	RTPBufferStatus readPackage(RTPPackageHandler &package);
private:
	int isPowerOfTwo(unsigned int x);
	void initializeRingBuffer(unsigned int contentSize, unsigned int rtpHeaderSize);
	RTPBufferPackage **ringBuffer;
	bool copyPlaybackDataIntoBuffer(RTPPackageHandler &package);
	void incrementReadPos();
	void lockMutex();
	void unlockMutex();
	unsigned int currentReadPos;

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
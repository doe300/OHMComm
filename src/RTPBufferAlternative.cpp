#include "RTPBufferAlternative.h"

RTPBufferAlternative::RTPBufferAlternative(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages) :
maxCapacity(maxCapacity), maxDelay(maxDelay), minBufferPackages(minBufferPackages), ringBuffer(nullptr)
{
	log = log2(maxCapacity);
	if (isPowerOfTwo(maxCapacity) == 0)
		throw "macCapacity must be a of power of two"; // TODO throw a OHMComm-Exception-Object

	#ifdef _WIN32
		bufferMutex = CreateMutex(nullptr, false, L"BufferMutex");
	#endif
}

int RTPBufferAlternative::isPowerOfTwo(unsigned int x)
{
	return ((x != 0) && ((x & (~x + 1)) == x));
}

RTPBufferStatus RTPBufferAlternative::addPackage(const RTPPackageHandler &package, unsigned int contentSize)
{
	// Calculate position in the ringBuffer for the package
	const RTPHeader *rtpHeader = package.getRTPPackageHeader();
	unsigned int packetPositionRingBuffer = rtpHeader->getSequenceNumber();
	// modulo operation by bit shifting
	if (packetPositionRingBuffer >= maxCapacity)
		packetPositionRingBuffer = (packetPositionRingBuffer << 32 - log) >> 32 - log;

	// Initiailize ringBuffer and set currentReadPos
	if (ringBuffer == nullptr)
	{
		initializeRingBuffer(package.getMaximumPayloadSize(), package.getRTPHeaderSize());
		currentReadPos = packetPositionRingBuffer;
		lastReadSeqNr = 0;
		amountOfPackages = 0;
	}

	// add packages to the buffer, which are not older than the current sequence number
	// For example the lastReadSeqNr was 55, then the package 52 or 51 are too old
	// These package will not be added to the buffer anymore
	unsigned int currentSeqNr = package.getRTPPackageHeader()->getSequenceNumber();

	if (currentSeqNr <= lastReadSeqNr)
		return RTPBufferStatus::RTP_BUFFER_PACKAGE_TO_OLD;

	// Add the packet to the ringBuffer
	lockMutex();
	auto headerBuffer = package.getRTPPackageHeader();
	auto rtpPackageDataBuffer = package.getRTPPackageData();
	ringBuffer[packetPositionRingBuffer]->addPacket(headerBuffer, rtpPackageDataBuffer);
	unlockMutex();

	// Overflow-Detection (check if the addPacket()-function did set the overflow flag)
	if (ringBuffer[packetPositionRingBuffer]->overflow())
		return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;

	// A new packet was added successfully, count the amount of added packets in the buffer
	amountOfPackages++;
	// Set flag, when enough packages have been added to the buffer
	if (amountOfPackages >= minBufferPackages)
		bufferContainsPackages = true;

	return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBufferAlternative::readPackage(RTPPackageHandler &package)
{
	// Make sure the buffer contains the minimum amount of packages
	if (bufferContainsPackages == false)
	{
		copySilencePackageIntoPackage(package);
		return RTPBufferStatus::RTP_BUFFER_IS_PUFFERING;
	}

	lockMutex();
	// Copy audio data from next package into the playbackBuffer of &package
	// return value indicates a underflow status (no new data/package has been read)
	bool underflow;

	if (isCurrentReadPosSet == false)
		setCurrentReadPos();

	if (amountOfUnderflowsInRow >= 20) // TODO magic number
	{
		underflow = copyNextPossiblePackageIntoPackage(package);
	}
	else
	{
		underflow = copyNextPackageIntoPackage(package);
	}
	unlockMutex();

	if (underflow)
	{
		lastReadSeqNr++;
		copySilencePackageIntoPackage(package);
		amountOfUnderflowsInRow++;
		return RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW;
	}

	// If there was no underflow, then data has been read from the buffer -> decrement currentPackagesCount
	if (amountOfPackages > 0)
		amountOfPackages--;
	if (amountOfPackages == 0) 
	{
		bufferContainsPackages = false;
		isCurrentReadPosSet = false;
	}
		

	amountOfUnderflowsInRow = 0;
	return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

unsigned int RTPBufferAlternative::getSize() const
{
    return amountOfPackages;
}

void RTPBufferAlternative::copySilencePackageIntoPackage(RTPPackageHandler &package)
{
	char* packageBuffer = (char*)package.getWorkBuffer();
	RTPHeader *currentHeaderData = (RTPHeader*)package.getRTPPackageHeader();
	currentHeaderData->setSequenceNumber(lastReadSeqNr);
	unsigned int rtpHeaderSize = package.getRTPHeaderSize();
	unsigned int payloadSize = package.getMaximumPayloadSize();

	// Copy data from buffer into package
	memcpy((char*)packageBuffer, currentHeaderData, rtpHeaderSize);
	memset((char*)packageBuffer + rtpHeaderSize, 0, payloadSize);
}

bool RTPBufferAlternative::copyNextPackageIntoPackage(RTPPackageHandler &package)
{
	// Find the current selected package from the buffer
	RTPBufferPackage *currentReadPackage = ringBuffer[currentReadPos];
	incrementReadPos();
	// Get the audioData from that selected package
	void *currentDataInBuffer = currentReadPackage->getPacketContent();
	// Get the header data from that selected package
	RTPHeader* currentHeaderData = currentReadPackage->getHeader();

	// Check if a underflow was detected (Package has already been read )
	bool underflow = currentReadPackage->underflow();
	// If underflow was detected -> abort
	if (underflow)
		return underflow;

	// Copy data from buffer into package
	void* packageBuffer = package.getWorkBuffer();
	int rtpHeaderSize = package.getRTPHeaderSize();
	int payloadSize = package.getMaximumPayloadSize();
	memcpy((char*)packageBuffer, currentHeaderData, rtpHeaderSize);
	memcpy((char*)packageBuffer + rtpHeaderSize, currentDataInBuffer, payloadSize);

	// set last read sequence number
	lastReadSeqNr = currentHeaderData->getSequenceNumber();
	return underflow;
}

bool RTPBufferAlternative::copyNextPossiblePackageIntoPackage(RTPPackageHandler &package)
{
	// the current selected package
	RTPBufferPackage *tmpPackage;
	void *audioPackageData;
	bool underflow;

	for (int i = 0; i < this->maxCapacity; i++)
	{
		lastReadSeqNr++;
		tmpPackage = ringBuffer[currentReadPos];
		incrementReadPos();

		// Get the audioData from the package in the buffer
		audioPackageData = tmpPackage->getPacketContent();
		// Check if a underflow was detected (Package has been read already)
		underflow = tmpPackage->underflow();

		if (underflow == false)
		{
			lastReadSeqNr = tmpPackage->getHeader()->getSequenceNumber();
			copyNextPackageIntoPackage(package);
			break;
		}
	}

	// When we reach here there are two cases:
	// underflow = true: This means we ran through the buffer once and I could not find a unread package
	// underflow = false: We have found a unread package

	if (underflow)
	{
		amountOfPackages = 0;
		lastReadSeqNr = 0;
		bufferContainsPackages = false;
		isCurrentReadPosSet = false;
	}

	return underflow;
}

void RTPBufferAlternative::setCurrentReadPos() 
{
	unsigned int smallestSeq = UINT_MAX;
	for (size_t i = 0; i < maxCapacity; i++)
	{
		unsigned int seqNr = ringBuffer[i]->getHeader()->getSequenceNumber();
		if (seqNr < smallestSeq && ringBuffer[i]->hasBeenReadAlready() == false)
		{
			smallestSeq = seqNr;
			currentReadPos = i;
		}
	}
	isCurrentReadPosSet = true;
}

// Initialize the whole buffer
void RTPBufferAlternative::initializeRingBuffer(unsigned int contentSize, unsigned int rtpHeaderSize)
{
	// Initialize a array of RTPBufferPackage pointers (on the heap)
	ringBuffer = new RTPBufferPackage*[maxCapacity];

	// Initialize every RTPBufferPackage in this array (on the heap)
	for (int i = 0; i < maxCapacity; i++)
	{
		ringBuffer[i] = new RTPBufferPackage(contentSize, rtpHeaderSize);
	}
}

void RTPBufferAlternative::incrementReadPos()
{
	currentReadPos++;
	if (currentReadPos >= maxCapacity)
		currentReadPos = 0;
}

void RTPBufferAlternative::lockMutex()
{
#ifdef _WIN32
	WaitForSingleObject(bufferMutex, INFINITE);
#else
	bufferMutex.lock();
#endif
}

void RTPBufferAlternative::unlockMutex()
{
#ifdef _WIN32
	ReleaseMutex(bufferMutex);
#else
	bufferMutex.unlock();
#endif
}

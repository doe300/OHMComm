#include "RTPBufferAlternative.h"

RTPBufferAlternative::RTPBufferAlternative(uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages) :
maxCapacity(maxCapacity), maxDelay(maxDelay), minBufferPackages(minBufferPackages), ringBuffer(nullptr)
{ 
	log = log2(maxCapacity);
	if (isPowerOfTwo(maxCapacity) == 0)
		throw "macCapacity must be a of power of two"; // TODO throw a real exception

	#ifdef _WIN32
		bufferMutex = CreateMutex(nullptr, false, L"BufferMutex");
	#endif
}

int RTPBufferAlternative::isPowerOfTwo(unsigned int x)
{
	return ((x != 0) && ((x & (~x + 1)) == x));
}

RTPBufferStatus RTPBufferAlternative::addPackage(RTPPackageHandler &package, unsigned int contentSize)
{
	// Calculate add-position in ringBuffer
	RTPHeader *rtpHeader = package.getRTPPackageHeader();
	unsigned int packetPositionRingBuffer = rtpHeader->sequence_number;
	
	if (packetPositionRingBuffer >= maxCapacity)
		packetPositionRingBuffer = (packetPositionRingBuffer << 32 - log) >> 32 - log;

	// Initiailize ringBuffer and set currentReadPos
	if (ringBuffer == nullptr) {
		initializeRingBuffer(contentSize, package.getRTPHeaderSize());	
		currentReadPos = packetPositionRingBuffer;
		bufferContainsPackages = true;
	}

	// Add the packet to the ringBuffer
	lockMutex();	
	ringBuffer[packetPositionRingBuffer]->addPacket(package.getRTPPackageHeader(), package.getRTPPackageData());
	unlockMutex();

	// Overflow-Detection
	if (ringBuffer[packetPositionRingBuffer]->overflow())
		return RTP_BUFFER_INPUT_OVERFLOW;

	packagesCount++;
	return RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBufferAlternative::readPackage(RTPPackageHandler &package)
{
	// Make sure the buffer contains any packages 
	// (RTAudio usally starts reading for incoming packets instead of processing outgoings)
	// (the processor thread tries to read, while the buffer is empty)
	if (bufferContainsPackages == false)
		return RTP_BUFFER_ALL_OKAY;

	// Wait until the amount of packages in the buffer are enough
	// Buffer a certain amount of packages before playback
	if (packagesCount <= minBufferPackages)
	{
		copySilencePackageIntoBuffer(package);
		return RTP_BUFFER_ALL_OKAY;
	}

	lockMutex();
	// Copy audio data from next available package into the playbackBuffer of &package
	// return value indicates a underflow status (no new data/package has been read)
	bool underflow = copyPlaybackDataIntoBuffer(package);
	unlockMutex();

	if (underflow)
		return RTP_BUFFER_OUTPUT_UNDERFLOW;
	
	// If there was no underflow, then data has been read from the buffer -> decrement packagesCount
	packagesCount--;
	return RTP_BUFFER_ALL_OKAY;
}

void RTPBufferAlternative::copySilencePackageIntoBuffer(RTPPackageHandler &package)
{
	char* packageBuffer = (char*)package.getWorkBuffer();
	int rtpHeaderSize = package.getRTPHeaderSize();

	// Write zeros into the buffer (silence package)
	memset(packageBuffer + rtpHeaderSize, 0, package.getActualPayloadSize());
}

bool RTPBufferAlternative::copyPlaybackDataIntoBuffer(RTPPackageHandler &package)
{
	// Find the current selected package from the buffer
	RTPBufferPackage *currentReadPackage = ringBuffer[currentReadPos];
	incrementReadPos();
	// Get the audioData from that selected package
	void *currentDataInBuffer = currentReadPackage->getPacketContent();
	// Check if a underflow was detected (Package has already been read )
	bool underflow = currentReadPackage->underflow();

	// When the current package has already been read then search for the next unread package
	if (underflow)
	{
		for (int i = 0; i < this->maxCapacity; i++)
		{
			currentReadPackage = ringBuffer[currentReadPos];
			incrementReadPos();

			// Get the audioData from the package in the buffer
			currentDataInBuffer = currentReadPackage->getPacketContent();
			// Check if a underflow was detected (Package has been read already)
			underflow = currentReadPackage->underflow();

			if (underflow == false)
				break;
		}
		// When we reach this codeline there are two cases:
		// underflow = true: This means we ran through the buffer once and I could not find a unread package
		// underflow = false: We have found a unread package
	}

	void* packageBuffer = package.getWorkBuffer();
	int rtpHeaderSize = package.getRTPHeaderSize();

	// Write data in playback (output) buffer
	memcpy((char*)packageBuffer + rtpHeaderSize, currentDataInBuffer, currentReadPackage->getPacketContentSize());
	
	return underflow;
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

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

	return RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBufferAlternative::readPackage(RTPPackageHandler &package)
{
	// Make sure the buffer contains any packages 
	// (RTAudio usally starts reading for incoming packets instead of processing outgoings)
	if (bufferContainsPackages == false)
		return RTP_BUFFER_ALL_OKAY;

	lockMutex();
	// Copy audio data from next available package into the playbackBuffer of package
	// return value indicates a underflow status
	bool underflow = copyPlaybackDataIntoBuffer(package);
	unlockMutex();

	if (underflow)
		return RTP_BUFFER_OUTPUT_UNDERFLOW;
	
	return RTP_BUFFER_ALL_OKAY;
}

bool RTPBufferAlternative::copyPlaybackDataIntoBuffer(RTPPackageHandler &package)
{
	RTPBufferPackage *currentReadPackage = ringBuffer[currentReadPos];
	incrementReadPos();

	// Get buffers and sizes
	void *currentDataInBuffer = currentReadPackage->getPacketContent();
	void* packageBuffer = package.getWorkBuffer();
	int rtpHeaderSize = package.getRTPHeaderSize();

	// Write data in playback (output) buffer
	memcpy((char*)packageBuffer + rtpHeaderSize, currentDataInBuffer, currentReadPackage->getPacketContentSize());
	bool underflow = currentReadPackage->underflow();
	
	return underflow;
}

void RTPBufferAlternative::initializeRingBuffer(unsigned int contentSize, unsigned int rtpHeaderSize)
{
	ringBuffer = new RTPBufferPackage*[maxCapacity];
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

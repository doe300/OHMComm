#include "ProcessorRTP.h"
#include "RTPPackage.h"

ProcessorRTP::ProcessorRTP(std::string name) : AudioProcessor(name) {}

void ProcessorRTP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, void *userData)
{
	// pack data into a rtp-package
}

void ProcessorRTP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, void *userData)
{
	// unpack data from a rtp-package
}
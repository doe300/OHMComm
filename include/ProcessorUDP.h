#ifndef PROCESSORUDP_H

#define	PROCESSORUDP_H
#include <string>
#include "AudioProcessor.h"
#include "UDPWrapper.h"

class ProcessorUDP : public UDPWrapper, AudioProcessor
{
public:
	ProcessorUDP(std::string name, struct NetworkConfiguration networkConfig);
	void processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
	void processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);
};
#endif
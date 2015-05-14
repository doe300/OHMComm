#ifndef PROCESSORUDP_H

#define	PROCESSORUDP_H
#include <string>
#include "AudioProcessor.h"
#include "UDPWrapper.h"

class ProcessorUDP : UDPWrapper, AudioProcessor
{
public:
	ProcessorUDP(std::string name, struct NetworkConfiguration networkConfig);
	void processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, void *userData = NULL);
	void processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, void *userData = NULL);
};
#endif
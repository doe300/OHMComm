#include "ProcessorUDP.h"

ProcessorUDP::ProcessorUDP(std::string name, struct NetworkConfiguration networkConfig) : UDPWrapper(networkConfig), AudioProcessor(name) {}

void ProcessorUDP::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, void *userData)
{
	this->sendDataNetworkWrapper(inputBuffer, inputBufferByteSize);
}

void ProcessorUDP::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, void *userData)
{
	this->recvDataNetworkWrapper(outputBuffer, outputBufferByteSize);
}

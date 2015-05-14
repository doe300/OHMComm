#ifndef AUDIOPROCESSOR_H
#define	AUDIOPROCESSOR_H

#include <iostream>
#include <string>

class AudioProcessor
{
public:
	AudioProcessor(std::string name);

	auto getName() const -> std::string;
	void setName(std::string name);

	/*
	 * The actual processing methods. processInputData is the counterpart of processInputData 
	 * (and also the other way around).
	 */
	virtual void processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, void *userData = NULL) = 0;
	virtual void processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, void *userData = NULL) = 0;
private:
	std::string name;
};



#endif
#ifndef AUDIOPROCESSOR_H
#define	AUDIOPROCESSOR_H

#include <iostream>
#include <string>

/*!
 * Abstract supertype for all classes used for intermediate handling of the input/output stream.
 * 
 * An implementation of this class may be used to encode/decode, filter or compress/decompress the input- and output-streams.
 * 
 * Processors can be chained, i.e. an output stream can be filtered -> encoded -> compressed.
 * The appertaining input-stream then will be decompressed -> decoded.
 */
class AudioProcessor
{
public:
	AudioProcessor(std::string name);

	auto getName() const -> std::string;
	void setName(std::string name);

        /*!
         * Overwrite this method, if this AudioProcessor needs configuration
         * 
         * For the style of configuration, see OhmComm.cpp
         */
        void configure();
        
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
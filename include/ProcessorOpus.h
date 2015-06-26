#ifndef PROCESSOROPUS_H

#define PROCESSOROPUS_H
#include "AudioProcessor.h"
#include "opus.h"
#include "stdint.h"
#include "configuration.h"

class ProcessorOpus : public AudioProcessor
{
public:

	//constructor
	ProcessorOpus(std::string name, int opusApplication);

	//deliver supported Audio Formats by Opus
	unsigned int getSupportedAudioFormats();
	//deliver supported Sample Rates by Opus
	unsigned int getSupportedSampleRates();

	//configure the Opus Processor, this creates OpusEncoder and OpusDecoderObject
	bool configure(AudioConfiguration audioConfig);

	//encodes the Data in inputBuffer(only Signed 16 bit PCM supported) and writes the encoded Data in encodedPacket 
	//returns lenght of encodedPacket in Bytes
	//TODO:	-integrate to override processInputData from AudioProcessor class
	//		-nBufferFrames automatic configuration; supported size one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//		-encodedPacket creation
	//		-maxPacketLength configuration
	unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);

	//decodes the Data in encodedPacket and writes it to the outputBuffer(only Signed 16 bit PCM supported)
	//returns number of decoded Samples
	//TODO: --integrate to override processOutputData from AudioProcessor class
	//		-nBufferFrames automatic configuration; supported size one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//		-encodedPacket access
	//		-lengthEncodedPacket access
	unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

	//destructor: destroys OpusEncoder and OpusDecoderObject
	~ProcessorOpus();

private:

	OpusEncoder *OpusEncoderObject;
	OpusDecoder *OpusDecoderObject;
	int OpusApplication;
	int ErrorCode;
	//TODO: delete lengthEncodedPacketWorkaround
	unsigned int lengthEncodedPacketWorkaround;

};
#endif
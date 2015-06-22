#ifndef PROCESSOROPUS_H

#define PROCESSOROPUS_H
#include "AudioProcessor.h"
#include "opus.h"
#include "stdint.h"
#include "configuration.h"

class ProcessorOpus : public AudioProcessor
{
public:

	//constructor: creates OpusEncoder and OpusDecoderObject
	ProcessorOpus(std::string name, int opusApplication, int ErrorCode);

	//encodes the Data in inputBuffer(only Signed 16 bit PCM supported) and writes the encoded Data in encodedPacket 
	//returns lenght of encodedPacket in Bytes
	//TODO:	-integrate to override processInputData from AudioProcessor class
	//		-nBufferFrames automatic configuration; supported size one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//		-encodedPacket creation
	//		-maxPacketLength configuration
	uint32_t encode(void *inputBuffer, unsigned int nBufferFrames, unsigned char *encodedPacket, opus_int32 maxPacketLength);

	//decodes the Data in encodedPacket and writes it to the outputBuffer(only Signed 16 bit PCM supported)
	//returns number of decoded Samples
	//TODO: --integrate to override processOutputData from AudioProcessor class
	//		-nBufferFrames automatic configuration; supported size one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//		-encodedPacket access
	//		-lengthEncodedPacket access
	uint32_t decode(unsigned char *encodedPacket, uint32_t lengthEncodedPacket, void *outputBuffer, unsigned int nBufferFrames);

	//destructor: destroys OpusEncoder and OpusDecoderObject
	~ProcessorOpus();

private:

	OpusEncoder *OpusEncoderObject;
	OpusDecoder *OpusDecoderObject;
	//TODO: discuss better methods to access audioConfiguration parameters (see Issue #32)
	AudioConfiguration audioConfiguration;
	opus_int32 sampleRate;
	int channels;

};
#endif
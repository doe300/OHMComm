#include "ProcessorOpus.h"
#include "opus.h"
#include "stdint.h"

class OpusEncoder : public ProcessorOpus
{
public:

	//constructor: creates OpusEncoderObject
	OpusEncoder(std::string name, int opusApplication, int encoderError);

	//encodes the Data in inputBuffer(only Signed 16 bit PCM supported) and writes the encoded Data in encodedPacket 
	//returns lenght of encodedPacket in Bytes
	//TODO:	-integrate to override processInputData from AudioProcessor class
	//		-nBufferFrames automatic configuration; supported size one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//		-encodedPacket creation
	//		-maxPacketLength configuration
	uint32_t encode(void *inputBuffer, unsigned int nBufferFrames, unsigned char *encodedPacket, opus_int32 maxPacketLength);

	//destructor: destroys OpusEncoderObject
	~OpusEncoder();

private:

	OpusEncoder *OpusEncoderObject;

};
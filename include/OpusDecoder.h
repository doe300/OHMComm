#include "ProcessorOpus.h"
#include "opus.h"
#include "stdint.h"

class OpusDecoder : public ProcessorOpus
{
public:

	//constructor: creates OpusDecoderObject
	OpusDecoder(std::string name, int decoderError);

	//decodes the Data in encodedPacket and writes it to the outputBuffer(only Signed 16 bit PCM supported)
	//returns number of decoded Samples
	//TODO: --integrate to override processOutputData from AudioProcessor class
	//		-nBufferFrames automatic configuration; supported size one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//		-encodedPacket access
	//		-lengthEncodedPacket access
	uint32_t decode(unsigned char *encodedPacket, uint32_t lengthEncodedPacket, void *outputBuffer, unsigned int nBufferFrames);

	//destructor: destroys OpusDecoderObject
	~OpusDecoder();

private:

	OpusDecoder *OpusDecoderObject;

};
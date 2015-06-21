#include "OpusDecoder.h"

OpusDecoder::OpusDecoder(std::string name, int encoderError) : ProcessorOpus(name)
{
	OpusDecoderObject = opus_decoder_create(sampleRate, channels, &encoderError);
}
uint32_t OpusDecoder::decode(unsigned char *encodedPacket, uint32_t lengthEncodedPacket, void *outputBuffer, unsigned int nBufferFrames)
{
	uint32_t numberOfDecodedSamples;
	numberOfDecodedSamples = opus_decode(OpusDecoderObject, encodedPacket, lengthEncodedPacket, (opus_int16 *)outputBuffer, nBufferFrames, 0);
	return numberOfDecodedSamples;
}
OpusDecoder::~OpusDecoder()
{
	opus_decoder_destroy(OpusDecoderObject);
}
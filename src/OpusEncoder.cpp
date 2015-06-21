#include "OpusEncoder.h"

OpusEncoder::OpusEncoder(std::string name, int opusApplication, int encoderError) : ProcessorOpus(name)
{
	OpusEncoderObject = opus_encoder_create(sampleRate, channels, opusApplication, &encoderError);
}
uint32_t OpusEncoder::encode(void *inputBuffer, unsigned int nBufferFrames, unsigned char *encodedPacket, opus_int32 maxPacketLength)
{
	uint32_t lengthEncodedPacket;
	lengthEncodedPacket = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, nBufferFrames, encodedPacket, maxPacketLength);
	return lengthEncodedPacket;
}
OpusEncoder::~OpusEncoder()
{
	opus_encoder_destroy(OpusEncoderObject);
}
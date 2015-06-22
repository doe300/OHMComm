#include "ProcessorOpus.h"

ProcessorOpus::ProcessorOpus(std::string name, int opusApplication, int ErrorCode) : AudioProcessor(name)
{
	//Opus supports this sampleRates: 8000, 12000, 16000, 24000, or 48000.
	if (audioConfiguration.sampleRate == 8000 || audioConfiguration.sampleRate == 12000 || audioConfiguration.sampleRate == 16000 || audioConfiguration.sampleRate == 24000 || audioConfiguration.sampleRate == 48000)
	{
		this->sampleRate = audioConfiguration.sampleRate;
	}
	else
	{
		std::cout << "SampleRate not supported by Opus! Opus supports this sampleRates: 8000, 12000, 16000, 24000, or 48000." << std::endl;
	}
	//Opus supports mono(1 channel) and stereo(2 channels) only.
	if (audioConfiguration.inputDeviceChannels == 1 || audioConfiguration.inputDeviceChannels == 2)
	{
		this->channels = audioConfiguration.inputDeviceChannels;
	}
	else
	{
		std::cout << "Device Channels not supported by Opus! Opus supports 1 or 2 Device Channels" << std::endl;
	}
	
	OpusEncoderObject = opus_encoder_create(sampleRate, channels, opusApplication, &ErrorCode);
	
	OpusDecoderObject = opus_decoder_create(sampleRate, channels, &ErrorCode);
	
}
uint32_t ProcessorOpus::encode(void *inputBuffer, unsigned int nBufferFrames, unsigned char *encodedPacket, opus_int32 maxPacketLength)
{
	uint32_t lengthEncodedPacket;
	lengthEncodedPacket = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, nBufferFrames, encodedPacket, maxPacketLength);
	return lengthEncodedPacket;
}
uint32_t ProcessorOpus::decode(unsigned char *encodedPacket, uint32_t lengthEncodedPacket, void *outputBuffer, unsigned int nBufferFrames)
{
	uint32_t numberOfDecodedSamples;
	numberOfDecodedSamples = opus_decode(OpusDecoderObject, encodedPacket, lengthEncodedPacket, (opus_int16 *)outputBuffer, nBufferFrames, 0);
	return numberOfDecodedSamples;
}
ProcessorOpus::~ProcessorOpus()
{
	opus_encoder_destroy(OpusEncoderObject);
	opus_decoder_destroy(OpusDecoderObject);
}
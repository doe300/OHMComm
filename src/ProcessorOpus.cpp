#include "ProcessorOpus.h"
#include <fstream> //stream output to file

ProcessorOpus::ProcessorOpus(std::string name, int opusApplication, int ErrorCode) : AudioProcessor(name)
{
	//how to get audioData Object(SampleRate, ChannelNumber)?
	
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
	// TODO: delete manuall selection of samplerate and channels
	OpusEncoderObject = opus_encoder_create(48000, 2, opusApplication, &ErrorCode);
	// TODO: delete manuall selection of samplerate and channels
	OpusDecoderObject = opus_decoder_create(48000, 2, &ErrorCode);
	
}
unsigned int ProcessorOpus::getSupportedAudioFormats()
{
	return AudioConfiguration::AUDIO_FORMAT_SINT16;
}
unsigned int ProcessorOpus::getSupportedSampleRates()
{
	//Opus supports this sampleRates: 8000, 12000, 16000, 24000, or 48000. = 1001111 binary in the format from configuration.h (every 1 stands for one supported format) = 79 decimal = 0x4F Hex
	unsigned int SupportedSampleRates = 79;
	return SupportedSampleRates;
}

unsigned char *EncodedPacketPointer = new unsigned char[4000];
unsigned int ProcessorOpus::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData)
{
	unsigned int lengthEncodedPacketInBytes;
	
	std::cout << userData->nBufferFrames << " [processInput]nBufferFrames should be 960" << std::endl;
	std::cout << userData->maxBufferSize << " [processInput]maxBufferSize" << std::endl;
	/*
	std::ofstream fileout("InputBuffer-before-encoding.raw", std::ios::out | std::ios::binary);
	fileout.write((char *)inputBuffer, userData->nBufferFrames);
	fileout.close();
	*/
	lengthEncodedPacketInBytes = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, userData->nBufferFrames, (unsigned char *)EncodedPacketPointer, userData->maxBufferSize);
	/*
	std::ofstream fileout2("InputBuffer-after-encoding.raw", std::ios::out | std::ios::binary);
	fileout2.write((char *)EncodedPacketPointer, lengthEncodedPacketInBytes);
	fileout2.close();
	*/
	std::cout << lengthEncodedPacketInBytes << " [processInput]lengthEncodedPacket" << std::endl;
	
	lengthEncodedPacketWorkaround = lengthEncodedPacketInBytes;
	return lengthEncodedPacketInBytes;
}



unsigned int ProcessorOpus::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
	unsigned int numberOfDecodedSamples;
	
	// -lengthEncodedPacketInBytes not correctly received, no variable for it
	// -outputBuffer isnt inputBuffer
	
	std::cout << userData->nBufferFrames << " [processOutput]nBufferFrames should be = " << lengthEncodedPacketWorkaround << " lengthEncodedPacket" << std::endl;
	std::cout << outputBufferByteSize << " [processOutput]outputBufferByteSize" << std::endl;
	std::cout << userData->maxBufferSize << " [processOutput]maxBufferSize" << std::endl;
	/*
	std::ofstream fileout3("outputBuffer-before-decoding.raw", std::ios::out | std::ios::binary);
	fileout3.write((char *)EncodedPacketPointer, lengthEncodedPacketWorkaround);
	fileout3.close();
	*/
	//outputBufferByteSize should be in samples
	numberOfDecodedSamples = opus_decode(OpusDecoderObject, (unsigned char *)EncodedPacketPointer, lengthEncodedPacketWorkaround, (opus_int16 *)outputBuffer, userData->maxBufferSize, 0);

	std::cout << numberOfDecodedSamples << " [processOutput]numberOfDecodedSamples should be 960" << std::endl;
	return numberOfDecodedSamples;
}


ProcessorOpus::~ProcessorOpus()
{
	opus_encoder_destroy(OpusEncoderObject);
	opus_decoder_destroy(OpusDecoderObject);
}
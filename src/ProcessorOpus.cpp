#include "ProcessorOpus.h"
#include <fstream> //stream output to file

ProcessorOpus::ProcessorOpus(std::string name, int opusApplication) : AudioProcessor(name)
{
	this->OpusApplication = opusApplication;	
}
unsigned int ProcessorOpus::getSupportedAudioFormats()
{
	//return AudioConfiguration::AUDIO_FORMAT_SINT16;
	std::cout << AudioConfiguration::AUDIO_FORMAT_SINT16 << " [getSupportedAudioFormats]AudioConfiguration::AUDIO_FORMAT_SINT16 should be 2" << std::endl;
	//TODO: fix Workaround acception all audio formats for now
	return AudioConfiguration::AUDIO_FORMAT_ALL;
}
unsigned int ProcessorOpus::getSupportedSampleRates()
{
	//Opus supports this sampleRates: 8000, 12000, 16000, 24000, or 48000. = 1001111 binary in the format from configuration.h (every 1 stands for one supported format) = 79 decimal = 0x4F Hex
	unsigned int SupportedSampleRates = AudioConfiguration::SAMPLE_RATE_8000|AudioConfiguration::SAMPLE_RATE_12000|AudioConfiguration::SAMPLE_RATE_16000|AudioConfiguration::SAMPLE_RATE_24000|AudioConfiguration::SAMPLE_RATE_48000;
	std::cout << SupportedSampleRates << " [getSupportedSampleRates]SupportedSampleRates should be 79" << std::endl;
	return SupportedSampleRates;
}
bool ProcessorOpus::configure(AudioConfiguration audioConfig)
{
	std::cout << audioConfig.sampleRate << " [configure]audioConfig.sampleRate should be 48000" << std::endl;
	std::cout << audioConfig.inputDeviceChannels << " [configure]audioConfig.inputDeviceChannels should be 2" << std::endl;
	std::cout << audioConfig.outputDeviceChannels << " [configure]audioConfig.outputDeviceChannels should be 2" << std::endl;
	OpusEncoderObject = opus_encoder_create(audioConfig.sampleRate, audioConfig.inputDeviceChannels, OpusApplication, &ErrorCode);
	OpusDecoderObject = opus_decoder_create(audioConfig.sampleRate, audioConfig.outputDeviceChannels, &ErrorCode);
	if (ErrorCode == OPUS_OK)
	{
		std::cout << "[configure]No Erros" << std::endl;
		return true;
	}
	else
	{
		if (ErrorCode == OPUS_ALLOC_FAIL)
		{
			std::cout << "[Opus-configure-Error]Memory allocation has failed." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_BAD_ARG)
		{
			std::cout << "[Opus-configure-Error]One or more invalid/out of range arguments." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_BUFFER_TOO_SMALL)
		{
			std::cout << "[Opus-configure-Error]The mode struct passed is invalid." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_INTERNAL_ERROR)
		{
			std::cout << "[Opus-configure-Error]An internal error was detected." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_INVALID_PACKET)
		{
			std::cout << "[Opus-configure-Error]The compressed data passed is corrupted." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_INVALID_STATE)
		{
			std::cout << "[Opus-configure-Error]An encoder or decoder structure is invalid or already freed." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_UNIMPLEMENTED)
		{
			std::cout << "[Opus-configure-Error]Invalid/unsupported request number. " << std::endl;
			return false;
		}
		else
		{
			std::cout << "[Opus-configure-Error]Unknown error " << std::endl;
			return false;
		}
	}
}
unsigned int zaehler1 = 0;
unsigned int ProcessorOpus::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData)
{
	unsigned int lengthEncodedPacketInBytes;
	
	//std::cout << userData->nBufferFrames << " [processInput]nBufferFrames should be 960" << std::endl;
	//std::cout << userData->maxBufferSize << " [processInput]maxBufferSize should be 3840" << std::endl;
	/*
	std::ofstream fileout("InputBuffer-before-encoding.raw", std::ios::out | std::ios::binary);
	fileout.write((char *)inputBuffer, userData->nBufferFrames);
	fileout.close();
	*/
	
	//std::cout << "[processInput]new encode" << zaehler1 << std::endl;
	lengthEncodedPacketInBytes = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, userData->nBufferFrames, (unsigned char *)inputBuffer, userData->maxBufferSize);
	/*
	std::ofstream fileout2("InputBuffer-after-encoding.raw", std::ios::out | std::ios::binary);
	fileout2.write((char *)EncodedPacketPointer, lengthEncodedPacketInBytes);
	fileout2.close();
	*/
	zaehler1++;
	std::cout << lengthEncodedPacketInBytes << " [processInput]lengthEncodedPacket " << zaehler1 << std::endl;
	lengthEncodedPacketWorkaround = lengthEncodedPacketInBytes;

	return lengthEncodedPacketInBytes;
}


unsigned int zaehler2 = 0;
unsigned int ProcessorOpus::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
	unsigned int numberOfDecodedSamples;
	
	// -lengthEncodedPacketInBytes not correctly received, no variable for it
	// -outputBuffer isnt inputBuffer
	zaehler2++;
	std::cout << outputBufferByteSize << " [processOutput]outputBufferByteSize should be = " << lengthEncodedPacketWorkaround << " lengthEncodedPacket " << zaehler2<< std::endl;
	//std::cout << userData->maxBufferSize << " [processOutput]maxBufferSize should be 3840" << std::endl;
	/*
	std::ofstream fileout3("outputBuffer-before-decoding.raw", std::ios::out | std::ios::binary);
	fileout3.write((char *)EncodedPacketPointer, lengthEncodedPacketWorkaround);
	fileout3.close();
	*/
	//outputBufferByteSize should be in samples
	
	
	numberOfDecodedSamples = opus_decode(OpusDecoderObject, (unsigned char *)outputBuffer, outputBufferByteSize, (opus_int16 *)outputBuffer, userData->maxBufferSize, 0);
	userData->nBufferFrames = numberOfDecodedSamples;
	//std::cout << numberOfDecodedSamples << " [processOutput]numberOfDecodedSamples should be 960" << std::endl;
	//TODO: find better way to find out Channel number ;(samples * sizeOfSample * channels)
	const unsigned int outputBytes = (numberOfDecodedSamples*sizeof(opus_int16) * 2);
	//std::cout << outputBytes << " [processOutput]outputBytes should be 3840" << std::endl;
	return outputBytes;
}


ProcessorOpus::~ProcessorOpus()
{
	opus_encoder_destroy(OpusEncoderObject);
	opus_decoder_destroy(OpusDecoderObject);
}

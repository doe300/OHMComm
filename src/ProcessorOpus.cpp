#include "ProcessorOpus.h"

ProcessorOpus::ProcessorOpus(std::string name, int opusApplication) : AudioProcessor(name)
{
	this->OpusApplication = opusApplication;	
}
unsigned int ProcessorOpus::getSupportedAudioFormats()
{
	return AudioConfiguration::AUDIO_FORMAT_SINT16;
}
unsigned int ProcessorOpus::getSupportedSampleRates()
{
	
	unsigned int SupportedSampleRates = AudioConfiguration::SAMPLE_RATE_8000|AudioConfiguration::SAMPLE_RATE_12000|AudioConfiguration::SAMPLE_RATE_16000|AudioConfiguration::SAMPLE_RATE_24000|AudioConfiguration::SAMPLE_RATE_48000;
	return SupportedSampleRates;
}
bool ProcessorOpus::configure(AudioConfiguration audioConfig)
{
	outputDeviceChannels = audioConfig.outputDeviceChannels;
	OpusEncoderObject = opus_encoder_create(audioConfig.sampleRate, audioConfig.inputDeviceChannels, OpusApplication, &ErrorCode);
	OpusDecoderObject = opus_decoder_create(audioConfig.sampleRate, audioConfig.outputDeviceChannels, &ErrorCode);
	if (ErrorCode == OPUS_OK)
	{
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
			std::cout << "[Opus-configure-Error]Unknown error." << std::endl;
			return false;
		}
	}
}
unsigned int ProcessorOpus::processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData)
{
	unsigned int lengthEncodedPacketInBytes;
	lengthEncodedPacketInBytes = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, userData->nBufferFrames, (unsigned char *)inputBuffer, userData->maxBufferSize);
	return lengthEncodedPacketInBytes;
}
unsigned int ProcessorOpus::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
	unsigned int numberOfDecodedSamples;
	numberOfDecodedSamples = opus_decode(OpusDecoderObject, (unsigned char *)outputBuffer, outputBufferByteSize, (opus_int16 *)outputBuffer, userData->maxBufferSize, 0);
	userData->nBufferFrames = numberOfDecodedSamples;
	const unsigned int outputBufferInBytes = (numberOfDecodedSamples*sizeof(opus_int16) * outputDeviceChannels);
	return outputBufferInBytes;
}
ProcessorOpus::~ProcessorOpus()
{
	opus_encoder_destroy(OpusEncoderObject);
	opus_decoder_destroy(OpusDecoderObject);
}

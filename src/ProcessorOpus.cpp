#include "ProcessorOpus.h"

ProcessorOpus::ProcessorOpus(std::string name, int opusApplication) : AudioProcessor(name)
{
	this->OpusApplication = opusApplication;	
}

ProcessorOpus::~ProcessorOpus()
{
	opus_encoder_destroy(OpusEncoderObject);
	opus_decoder_destroy(OpusDecoderObject);
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

std::vector<int> ProcessorOpus::getSupportedBufferSizes(uint32_t sampleRate)
{
	if (sampleRate == 8000)
	{
		return std::vector<int>({ 20, 40, 80, 160, 320, 480 });
	}
	else if (sampleRate == 12000)
	{
		return std::vector<int>({ 30, 60, 120, 240, 480, 720 });
	}
	else if (sampleRate == 16000)
	{
		return std::vector<int>({ 40, 80, 160, 320, 640, 960 });
	}
	else if (sampleRate == 24000)
	{
		return std::vector<int>({ 60, 120, 240, 480, 960, 1440 });
	}
	else if (sampleRate == 48000)
	{
		return std::vector<int>({ 120, 240, 480, 960, 1920, 2880 });
	}
	else
	{
		std::cerr << "[Opus-getSupportedBufferSizes-Error]No supported buffer size could be found." << std::endl;
		return std::vector<int>({ });
	}
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
			std::cerr << "[Opus-configure-Error]Memory allocation has failed." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_BAD_ARG)
		{
			std::cerr << "[Opus-configure-Error]One or more invalid/out of range arguments." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_BUFFER_TOO_SMALL)
		{
			std::cerr << "[Opus-configure-Error]The mode struct passed is invalid." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_INTERNAL_ERROR)
		{
			std::cerr << "[Opus-configure-Error]An internal error was detected." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_INVALID_PACKET)
		{
			std::cerr << "[Opus-configure-Error]The compressed data passed is corrupted." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_INVALID_STATE)
		{
			std::cerr << "[Opus-configure-Error]An encoder or decoder structure is invalid or already freed." << std::endl;
			return false;
		}
		else if (ErrorCode == OPUS_UNIMPLEMENTED)
		{
			std::cerr << "[Opus-configure-Error]Invalid/unsupported request number. " << std::endl;
			return false;
		}
		else
		{
			std::cerr << "[Opus-configure-Error]Unknown error." << std::endl;
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


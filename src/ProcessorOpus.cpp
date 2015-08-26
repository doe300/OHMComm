#include "ProcessorOpus.h"

ProcessorOpus::ProcessorOpus(const std::string name, int opusApplication) : AudioProcessor(name)
{
	this->OpusApplication = opusApplication;
}

ProcessorOpus::~ProcessorOpus()
{
	opus_encoder_destroy(OpusEncoderObject);
	opus_decoder_destroy(OpusDecoderObject);
}

unsigned int ProcessorOpus::getSupportedAudioFormats() const
{
	return AudioConfiguration::AUDIO_FORMAT_SINT16|AudioConfiguration::AUDIO_FORMAT_FLOAT32;
}

unsigned int ProcessorOpus::getSupportedSampleRates() const
{
	unsigned int SupportedSampleRates = AudioConfiguration::SAMPLE_RATE_8000|AudioConfiguration::SAMPLE_RATE_12000|AudioConfiguration::SAMPLE_RATE_16000|AudioConfiguration::SAMPLE_RATE_24000|AudioConfiguration::SAMPLE_RATE_48000;
	return SupportedSampleRates;
}

const std::vector<int> ProcessorOpus::getSupportedBufferSizes(unsigned int sampleRate) const
{
	if (sampleRate == 8000)
	{
		return std::vector<int>({ 480, 320, 160, 80, 40, 20 });
	}
	else if (sampleRate == 12000)
	{
		return std::vector<int>({ 720, 480, 240, 120, 60, 30 });
	}
	else if (sampleRate == 16000)
	{
		return std::vector<int>({ 960, 640, 320, 160, 80, 40 });
	}
	else if (sampleRate == 24000)
	{
		return std::vector<int>({ 1440, 960, 480, 240, 120, 60 });
	}
	else if (sampleRate == 48000)
	{
		return std::vector<int>({ 960, 1920, 2880, 480, 240, 120 });
	}
	else
	{
		std::cerr << "[Opus-getSupportedBufferSizes-Error]No supported buffer size could be found." << std::endl;
		return std::vector<int>({ });
	}
}

bool ProcessorOpus::configure(const AudioConfiguration& audioConfig)
{
	outputDeviceChannels = audioConfig.outputDeviceChannels;
	rtaudioFormat = audioConfig.audioFormatFlag;

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
	unsigned int lengthEncodedPacketInBytes = 0;
	if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_SINT16)
	{
		lengthEncodedPacketInBytes = opus_encode(OpusEncoderObject, (opus_int16 *)inputBuffer, userData->nBufferFrames, (unsigned char *)inputBuffer, userData->maxBufferSize);
		return lengthEncodedPacketInBytes;
	}
	else if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_FLOAT32)
	{
		lengthEncodedPacketInBytes = opus_encode_float(OpusEncoderObject, (const float *)inputBuffer, userData->nBufferFrames, (unsigned char *)inputBuffer, userData->maxBufferSize);
		return lengthEncodedPacketInBytes;
	}
	else
	{
		std::cerr << "[Opus-processInputData-Error]No matching encoder found, AudioFormat possibly not supported" << std::endl;
        return 0;
	}
}

unsigned int ProcessorOpus::processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData)
{
	unsigned int numberOfDecodedSamples = 0;
	if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_SINT16)
	{
		numberOfDecodedSamples = opus_decode(OpusDecoderObject, (unsigned char *)outputBuffer, outputBufferByteSize, (opus_int16 *)outputBuffer, userData->maxBufferSize, 0);
		userData->nBufferFrames = numberOfDecodedSamples;
		const unsigned int outputBufferInBytes = (numberOfDecodedSamples * sizeof(opus_int16) * outputDeviceChannels);
		return outputBufferInBytes;
	}
	else if (rtaudioFormat == AudioConfiguration::AUDIO_FORMAT_FLOAT32)
	{
		numberOfDecodedSamples = opus_decode_float(OpusDecoderObject, (const unsigned char *)outputBuffer, outputBufferByteSize, (float *)outputBuffer, userData->maxBufferSize, 0);
		userData->nBufferFrames = numberOfDecodedSamples;
		const unsigned int outputBufferInBytes = (numberOfDecodedSamples * sizeof(float) * outputDeviceChannels);
		return outputBufferInBytes;
	}
	else
	{
		std::cerr << "[Opus-processOutputData-Error]No matching decoder found, AudioFormat possibly not supported" << std::endl;
        return 0;
	}
}


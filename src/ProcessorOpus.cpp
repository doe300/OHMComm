#include "ProcessorOpus.h"

ProcessorOpus::ProcessorOpus(std::string name) : AudioProcessor(name)
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
}
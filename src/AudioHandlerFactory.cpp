#include "AudioHandlerFactory.h"

auto AudioHandlerFactory::getAudioHandler(std::string name, AudioConfiguration &audioConfig) ->std::unique_ptr<AudioHandler>
{
	std::transform(name.begin(), name.end(), name.begin(), toupper);
	if (name == "RTAUDIOWRAPPER")
	{
		std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper(audioConfig));
		return std::move(rtaudiowrapper);
	}
}



auto AudioHandlerFactory::getAudioHandler(std::string name) ->std::unique_ptr<AudioHandler>
{
	std::transform(name.begin(), name.end(), name.begin(), toupper);
	if (name == "RTAUDIOWRAPPER")
	{
		std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper);
		return std::move(rtaudiowrapper);
	}
}

auto AudioHandlerFactory::stringToUpperCase(const std::string& s) -> std::string
{
	std::string result;

	std::locale loc;
	for (unsigned int i = 0; i < s.length(); ++i)
	{
		result += std::toupper(s.at(i), loc);
	}

	return result;
}
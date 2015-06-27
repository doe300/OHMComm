#include "AudioHandlerFactory.h"

auto AudioHandlerFactory::getAudioHandler(std::string name, AudioConfiguration &audioConfig) ->std::unique_ptr<AudioHandler>
{
	name = stringToUpperCase(name);
        #ifdef RTAUDIOWRAPPER_H
	if (name == "RTAUDIOWRAPPER")
	{
		std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper(audioConfig));
		return std::move(rtaudiowrapper);
	}
        #endif
}

auto AudioHandlerFactory::getAudioHandler(std::string name) ->std::unique_ptr<AudioHandler>
{
	name = stringToUpperCase(name);
        #ifdef RTAUDIOWRAPPER_H
	if (name == "RTAUDIOWRAPPER")
	{
		std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper);
		return std::move(rtaudiowrapper);
	}
        #endif
}

const std::vector<std::string> AudioHandlerFactory::getAudioHandlerNames()
{
    std::vector<std::string> handlerNames;
    #ifdef RTAUDIOWRAPPER_H
    handlerNames.push_back("RtAudioWrapper");
    #endif
    return handlerNames;
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
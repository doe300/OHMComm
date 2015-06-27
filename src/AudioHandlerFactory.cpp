#include "AudioHandlerFactory.h"

//Initialize names
const std::string AudioHandlerFactory::RTAUDIO_WRAPPER = "RtAudioWrapper";

auto AudioHandlerFactory::getAudioHandler(std::string name, AudioConfiguration &audioConfig) ->std::unique_ptr<AudioHandler>
{
        #ifdef RTAUDIOWRAPPER_H
	if (name == RTAUDIO_WRAPPER)
	{
		std::unique_ptr<RtAudioWrapper> rtaudiowrapper(new RtAudioWrapper(audioConfig));
		return std::move(rtaudiowrapper);
	}
        #endif
}

auto AudioHandlerFactory::getAudioHandler(std::string name) ->std::unique_ptr<AudioHandler>
{
        #ifdef RTAUDIOWRAPPER_H
	if (name == RTAUDIO_WRAPPER)
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
    handlerNames.push_back(RTAUDIO_WRAPPER);
    #endif
    return handlerNames;
}

std::string AudioHandlerFactory::getDefaultAudioHandlerName()
{
    #ifdef RTAUDIOWRAPPER_H
    return RTAUDIO_WRAPPER;
    #endif
}

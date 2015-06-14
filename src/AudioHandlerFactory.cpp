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
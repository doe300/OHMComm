#include "AudioProcessor.h"
#include "opus.h"
#include "configuration.h"

class ProcessorOpus : public AudioProcessor
{
public:

	ProcessorOpus(std::string name);

protected:

	//TODO: discuss better methods to access audioConfiguration parameters, perhaps over AudioProcessor?
	AudioConfiguration audioConfiguration;
	opus_int32 sampleRate;
	int channels;

};
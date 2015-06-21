#include "AudioProcessor.h"
#include "AudioHandler.h"
#include "opus.h"

class ProcessorOpus : public AudioProcessor
{
public:

	ProcessorOpus(std::string name);

protected:

	opus_int32 sampleRate;
	int channels;

};
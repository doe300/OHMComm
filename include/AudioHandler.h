#ifndef AUDIOIO_H
#define	AUDIOIO_H

#include "AudioProcessor.h"
#include "configuration.h"
#include <vector>
#include <memory>

typedef std::unique_ptr<AudioProcessor> ptrAudioProcessor;

/*!
 * Base class for Audio framework.
 * 
 * Implementations of this class wrap a specific audio library
 */
class AudioHandler
{
public:
	virtual void startRecordingMode() = 0;
	virtual void startPlaybackMode() = 0;
	virtual void startDuplexMode() = 0;

	virtual void setConfiguration(const AudioConfiguration &audioConfiguration) = 0;
	virtual void suspend() = 0; // suspends the stream
	virtual void resume() = 0; // resume the stream
	virtual void stop() = 0; // close the whole communication process
	virtual void reset() = 0; // stop and reset audioConfiguration
	virtual void playData(void *playbackData, unsigned int size) = 0;
	virtual void setDefaultAudioConfig() = 0; // will load the default-config
	virtual auto prepare() -> bool = 0; // prepare everything for execution (allocates memory if needed, configure audioprocessors)
	virtual auto getBufferSize() -> unsigned int = 0;

	void printAudioProcessorOrder(std::ostream& outputStream = std::cout) const;
	auto addProcessor(AudioProcessor *audioProcessor) -> bool;
	auto removeAudioProcessor(AudioProcessor *audioProcessor) -> bool;
	auto removeAudioProcessor(std::string nameOfAudioProcessor) -> bool;
	auto clearAudioProcessors() -> bool;
        
	auto hasAudioProcessor(AudioProcessor *audioProcessor) const -> bool;
	auto hasAudioProcessor(std::string nameOfAudioProcessor) const -> bool;
	auto getAudioConfiguration() -> AudioConfiguration;

	auto IsAudioConfigSet() const -> bool;
	auto IsPrepared() const -> bool;

protected:
	bool isAudioConfigSet = false;
	bool isPrepared = false;


	std::vector<AudioProcessor*> audioProcessors;
	AudioConfiguration audioConfiguration;	
	void processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, StreamData *streamData);
	void processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, StreamData *streamData);

	/*!
	* Calls AudioProcessor#configure() for all registered processors
	*/
	auto configureAudioProcessors() -> bool;
};

#endif
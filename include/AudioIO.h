#ifndef AUDIOIO_H
#define	AUDIOIO_H

#include "AudioProcessor.h"
#include "configuration.h"
#include <vector>
#include <memory>

typedef std::unique_ptr<AudioProcessor> ptrAudioProcessor;

class AudioIO
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

	void printAudioProcessorOrder() const;
	void addProcessor(AudioProcessor *audioProcessor);
	void removeAudioProcessor(AudioProcessor *audioProcessor);
	void removeAudioProcessor(std::string nameOfAudioProcessor);
	void clearAudioProcessors();
        
        /*!
         * Calls AudioProcessor#configure() for all registered processors
         */
        void configureAudioProcessors();

protected:
	std::vector<AudioProcessor*> audioProcessors;
	auto hasAudioProcessor(AudioProcessor *audioProcessor) const -> bool;
	auto hasAudioProcessor(std::string nameOfAudioProcessor) const -> bool;
	void processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, void *userData1 = NULL, void *userData2 = NULL);
	void processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, void *userData1 = NULL, void *userData2 = NULL);
};

#endif
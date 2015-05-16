#ifndef RTAUDIOWRAPPER_H

#define	RTAUDIOWRAPPER_H

#include "RtAudio.h"
#include "AudioIO.h"
#include "math.h" // ceiling
#include <memory> // unique_ptr
#include <string.h> //memcpy

#ifdef _WIN32
#include <windows.h> // semaphores
#else
/* TODO, two linux semaphores needed */
#endif



class RtAudioWrapper: public AudioIO
{
public:
	/* object generator methods (equals constructor) */
	static auto getNewAudioIO()->std::unique_ptr<AudioIO>;
	static auto getNewAudioIO(const AudioConfiguration &audioConfig)->std::unique_ptr<AudioIO>;

	/* deny copies with the copy constructor */
	RtAudioWrapper(const RtAudioWrapper & copy) = delete;
	
	/* AudioIO methods */
	void startRecordingMode();
	void startPlaybackMode();
	void startDuplexMode();
	void setConfiguration(const AudioConfiguration &audioConfiguration);
	/* suspends the stream */
	void suspend();
	/* resume the stream (only possible if the stream was not stopped) */
	void resume();
	/* close the whole communication process */
	void stop(); 
	/* stop() and resets audioConfiguration */
	void reset();
	/* This is a blocking function, do NOT call this function within the RtAudio callback */
	void playData(void *playbackData, unsigned int size); 

	/* Callbacks */
	auto callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData) -> int;
	static auto callbackHelper(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *rtAudioWrapperObject) -> int;	

private:
	/* private Constructors */
	RtAudioWrapper();
	RtAudioWrapper(const AudioConfiguration &audioConfig);

	/* variables for the "void playData(...)" function
	 * to enable synchronizing between rtaudio- and main-thread */
	#ifdef _WIN32
	HANDLE semaphore_waitForMainThread;
	HANDLE semaphore_waitForWorkerThread;
	#else
	/* TODO, two linux semaphores needed */
	#endif
	void *bufferAudioOutput;
	bool BufferAudioOutHasData = false;


	RtAudio rtaudio;
	RtAudio::StreamParameters input, output;
	AudioConfiguration audioConfiguration;
	bool isAudioConfigSet = false;
	unsigned int outputBufferByteSize = {0};
	unsigned int inputBufferByteSize = {0};


	/* preparing for a openstream call */
	void initRtAudioStreamParameters(); 

	/* will load the default-config */
	void loadDefaultAudioConfig();

	/* returns the size of a frame in bytes */
	auto getAudioFormatByteSize(RtAudioFormat rtaudioFormat) -> int;

	/* returns the actual output framesize in bytes */
	auto getOutputFrameSize() -> int;

	/* returns the actual input framesize in bytes */
	auto getInputFrameSize() -> int;

	/* returns the best supported rtaudioformat*/
	auto autoSelectAudioFormat(RtAudioFormat supportedFormats) -> RtAudioFormat;

};

#endif
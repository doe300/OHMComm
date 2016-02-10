/* 
 * File:   PortAudioWrapper.h
 * Author: daniel
 *
 * Created on February 10, 2016, 1:21 PM
 */

#ifdef PORTAUDIO_HEADER //Only compile, if PortAudio is linked
#ifndef PORTAUDIOWRAPPER_H
#define	PORTAUDIOWRAPPER_H

#include "AudioHandler.h"
#include PORTAUDIO_HEADER

class PortAudioWrapper : public AudioHandler
{
public:
    PortAudioWrapper();
    PortAudioWrapper(const AudioConfiguration &audioConfig);

    ~PortAudioWrapper();

    /* deny copies with the copy constructor */
    PortAudioWrapper(const PortAudioWrapper & copy) = delete;

    void startRecordingMode();
    void startPlaybackMode();
    void startDuplexMode();
    void setConfiguration(const AudioConfiguration &audioConfiguration);
    void suspend();
    void resume();
    void stop();
    void reset();
    void setDefaultAudioConfig();
    auto prepare(const std::shared_ptr<ConfigurationMode> configMode) -> bool;
    auto getBufferSize() -> unsigned int;
    
    const std::vector<AudioDevice>& getAudioDevices();
private:
    StreamData* streamData;
    PaStreamParameters outputParams;
    PaStreamParameters inputParams;
    PaStream* stream;
    unsigned int bufferSize;
    PaTime streamStartTime = 0;
    
    static inline unsigned int throwOnError(PaError error)
    {
        //error-codes are all negative
        if(error < 0)
            throw std::runtime_error(Pa_GetErrorText(error));
        return (unsigned int) error;
    }
    
    static std::vector<unsigned int> getSupportedSampleRates(const PaDeviceIndex deviceIndex);
    
    static int callbackHelper(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData );
    
    int callback(void *inputBuffer, void *outputBuffer, unsigned long frameCount, const double streamTime, PaStreamCallbackFlags statusFlags);
    
    bool initStreamParameters();
    
    static PaSampleFormat mapSampleFormat(const unsigned int sampleFormatFlag);
};

#endif	/* PORTAUDIOWRAPPER_H */
#endif /* PORTAUDIO_HEADER */
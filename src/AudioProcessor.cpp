/* 
 * File:   AudioProcessor.cpp
 * Author: daniel
 * 
 * Created on March 29, 2015, 1:36 PM
 */

#include "AudioProcessor.h"

/*
 * Dummy implementation of AudioProcessor
 */

AudioProcessor::AudioProcessor(AudioProcessor *underlying, const void *settings)
{
    AudioProcessor::underlying = underlying;
    AudioProcessor::settings = settings;
}

AudioProcessor::AudioProcessor(const AudioProcessor& orig)
{
    AudioProcessor::underlying = orig.underlying;
    AudioProcessor::settings = orig.settings;
}

AudioProcessor::~AudioProcessor()
{
}

int AudioProcessor::process(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
    //dummy implementation - pass through to underlying processor
    return underlying->process(outputBuffer, inputBuffer, nFrames, streamTime, status, userData);
}


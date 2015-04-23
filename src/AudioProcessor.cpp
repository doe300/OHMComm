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

AudioProcessor::AudioProcessor()
{
    nextInChain = NULL;
}

AudioProcessor::AudioProcessor(const AudioProcessor& orig)
{
    nextInChain = orig.nextInChain;
}

AudioProcessor::~AudioProcessor()
{
}

int AudioProcessor::process(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
    //dummy implementation - pass through to underlying processor
    return nextInChain->process(outputBuffer, inputBuffer, nFrames, streamTime, status, userData);
}

void AudioProcessor::configure()
{
    //dummy implementation - do nothing
}

void AudioProcessor::setNextInChain(AudioProcessor* nextInChain)
{
    this->nextInChain = nextInChain;
}

AudioProcessor* AudioProcessor::getNextInChain()
{
    return nextInChain;
}



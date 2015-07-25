/* 
 * File:   AudioProcessorFactory.cpp
 * Author: daniel
 * 
 * Created on June 27, 2015, 10:15 AM
 */

#include "AudioProcessorFactory.h"

#include "ProcessorOpus.h"
#include "ProcessorWAV.h"
#include "ProfilingAudioProcessor.h"

const std::string AudioProcessorFactory::OPUS_CODEC = "Opus-Codec";
const std::string AudioProcessorFactory::WAV_WRITER = "wav-Writer";

AudioProcessor* AudioProcessorFactory::getAudioProcessor(std::string name, bool createProfiler)
{
    AudioProcessor* processor = nullptr;
    #ifdef PROCESSOROPUS_H
    if(name == OPUS_CODEC)
    {
        processor = new ProcessorOpus(OPUS_CODEC, OPUS_APPLICATION_VOIP);
    }
    #endif
    #ifdef PROCESSORWAV_H
    if(name == WAV_WRITER)
    {
        processor = new ProcessorWAV();
    }
    #endif
    if(processor != nullptr)
    {
        if(createProfiler)
        {
            return new ProfilingAudioProcessor(processor);
        }
        return processor;
    }
    throw std::invalid_argument("No AudioProcessor for the given name");
}

const std::vector<std::string> AudioProcessorFactory::getAudioProcessorNames()
{
    std::vector<std::string> processorNames;
    #ifdef PROCESSOROPUS_H
    processorNames.push_back(OPUS_CODEC);
    #endif
    #ifdef PROCESSORWAV_H
    processorNames.push_back(WAV_WRITER);
    #endif
    return processorNames;
}


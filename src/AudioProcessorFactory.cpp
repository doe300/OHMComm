/*
 * File:   AudioProcessorFactory.cpp
 * Author: daniel
 *
 * Created on June 27, 2015, 10:15 AM
 */

#include "AudioProcessorFactory.h"

#include "codecs/ProcessorOpus.h"
#include "ProcessorWAV.h"
#include "codecs/ProcessorALaw.h"
#include "codecs/ProcessorMuLaw.h"
#include "filters/GainControl.h"
#include "ProfilingAudioProcessor.h"

const std::string AudioProcessorFactory::OPUS_CODEC = "Opus-Codec";
const std::string AudioProcessorFactory::WAV_WRITER = "wav-Writer";
const std::string AudioProcessorFactory::G711_PCMA = "A-law";
const std::string AudioProcessorFactory::G711_PCMU = "mu-law";
const std::string AudioProcessorFactory::GAIN_CONTROL = "Gain Control";

AudioProcessor* AudioProcessorFactory::getAudioProcessor(const std::string name, bool createProfiler)
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
        processor = new ProcessorWAV(WAV_WRITER);
    }
    #endif
    #ifdef PROCESSORALAW_H
    if(name == G711_PCMA)
    {
        processor = new ProcessorALaw(G711_PCMA);
    }
    #endif
    #ifdef PROCESSORMULAW_H
    if(name == G711_PCMU)
    {
        processor = new ProcessorMuLaw(G711_PCMU);
    }
    #endif
    #ifdef GAINCONTROL_H
    if(name == GAIN_CONTROL)
    {
        processor = new GainControl(GAIN_CONTROL);
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
    #ifdef PROCESSORALAW_H
    processorNames.push_back(G711_PCMA);
    #endif
    #ifdef PROCESSORMULAW_H
    processorNames.push_back(G711_PCMU);
    #endif
    #ifdef GAINCONTROL_H
    processorNames.push_back(GAIN_CONTROL);
    #endif
    return processorNames;
}


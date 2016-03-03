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
#include "processors/GainControl.h"
#include "ProfilingAudioProcessor.h"
#include "codecs/ProcessoriLBC.h"
#include "codecs/AMRCodec.h"
#include "codecs/SpeexCodec.h"

const std::string AudioProcessorFactory::OPUS_CODEC = "Opus-Codec";
const std::string AudioProcessorFactory::WAV_WRITER = "wav-Writer";
const std::string AudioProcessorFactory::G711_PCMA = "A-law";
const std::string AudioProcessorFactory::G711_PCMU = "mu-law";
const std::string AudioProcessorFactory::GAIN_CONTROL = "Gain Control";
const std::string AudioProcessorFactory::ILBC_CODEC = "iLBC-Codec";
const std::string AudioProcessorFactory::AMR_CODEC = "AMR-Codec";
const std::string AudioProcessorFactory::SPEEX_CODEC = "Speex-Codec";

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
    #ifdef PROCESSORILBC_H
    if(name == ILBC_CODEC)
        processor = new ProcessoriLBC(ILBC_CODEC);
    #endif
    #ifdef AMRCODEC_H
    if(name == AMR_CODEC)
        processor = new AMRCodec(AMR_CODEC);
    #endif
    #ifdef SPEEXCODEC_H
    if(name == SPEEX_CODEC)
        return new SpeexCodec(SPEEX_CODEC);
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
    #ifdef PROCESSORILBC_H
    processorNames.push_back(ILBC_CODEC);
    #endif
    #ifdef AMRCODEC_H
    processorNames.push_back(AMR_CODEC);
    #endif
    #ifdef SPEEXCODEC_H
    processorNames.push_back(SPEEX_CODEC);
    #endif
    return processorNames;
}


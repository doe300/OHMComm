/*
 * File:   AudioProcessorFactory.cpp
 * Author: daniel
 *
 * Created on June 27, 2015, 10:15 AM
 */

#include "processors/AudioProcessorFactory.h"

#include "codecs/OpusCodec.h"
#include "processors/ProcessorWAV.h"
#include "codecs/G711Alaw.h"
#include "codecs/G711Mulaw.h"
#include "processors/GainControl.h"
#include "processors/ProfilingAudioProcessor.h"
#include "codecs/ProcessoriLBC.h"
#include "codecs/GSMCodec.h"
#include "codecs/AMRCodec.h"

using namespace ohmcomm;

const std::string AudioProcessorFactory::OPUS_CODEC = "Opus-Codec";
const std::string AudioProcessorFactory::WAV_WRITER = "wav-Writer";
const std::string AudioProcessorFactory::G711_PCMA = "A-law";
const std::string AudioProcessorFactory::G711_PCMU = "mu-law";
const std::string AudioProcessorFactory::GAIN_CONTROL = "Gain Control";
const std::string AudioProcessorFactory::ILBC_CODEC = "iLBC-Codec";
const std::string AudioProcessorFactory::GSM_CODEC = "GSM";
const std::string AudioProcessorFactory::AMR_CODEC = "AMR-Codec";

AudioProcessor* AudioProcessorFactory::getAudioProcessor(const std::string name, bool createProfiler)
{
    AudioProcessor* processor = nullptr;
    #ifdef OHMCOMM_OPUS_H
    if(name == OPUS_CODEC)
    {
        processor = new codecs::OpusCodec(OPUS_CODEC);
    }
    #endif
    #ifdef PROCESSORWAV_H
    if(name == WAV_WRITER)
    {
        processor = new ProcessorWAV(WAV_WRITER);
    }
    #endif
    #ifdef OHMCOMM_G711ALAW_H
    if(name == G711_PCMA)
    {
        processor = new codecs::G711Alaw(G711_PCMA);
    }
    #endif
    #ifdef OHMCOMM_G711MULAW_H
    if(name == G711_PCMU)
    {
        processor = new codecs::G711Mulaw(G711_PCMU);
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
        processor = new codecs::ProcessoriLBC(ILBC_CODEC);
    #endif
    #ifdef GSMCODEC_H
    if(name == GSM_CODEC)
        processor = new codecs::GSMCodec(GSM_CODEC);
    #endif
    #ifdef AMRCODEC_H
    if(name == AMR_CODEC)
        processor = new codecs::AMRCodec(AMR_CODEC);
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
    #ifdef OHMCOMM_OPUS_H
    processorNames.push_back(OPUS_CODEC);
    #endif
    #ifdef PROCESSORWAV_H
    processorNames.push_back(WAV_WRITER);
    #endif
    #ifdef OHMCOMM_G711ALAW_H
    processorNames.push_back(G711_PCMA);
    #endif
    #ifdef OHMCOMM_G711MULAW_H
    processorNames.push_back(G711_PCMU);
    #endif
    #ifdef GAINCONTROL_H
    processorNames.push_back(GAIN_CONTROL);
    #endif
    #ifdef PROCESSORILBC_H
    processorNames.push_back(ILBC_CODEC);
    #endif
    #ifdef GSMCODEC_H
    processorNames.push_back(GSM_CODEC);
    #endif
    #ifdef AMRCODEC_H
    processorNames.push_back(AMR_CODEC);
    #endif
    return processorNames;
}


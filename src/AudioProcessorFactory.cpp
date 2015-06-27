/* 
 * File:   AudioProcessorFactory.cpp
 * Author: daniel
 * 
 * Created on June 27, 2015, 10:15 AM
 */

#include "AudioProcessorFactory.h"

const std::string AudioProcessorFactory::OPUS_CODEC = "Opus-Codec";

AudioProcessor* AudioProcessorFactory::getAudioProcessor(std::string name)
{
    #ifdef PROCESSOROPUS_H
    if(name == OPUS_CODEC)
    {
        return new ProcessorOpus(OPUS_CODEC, OPUS_APPLICATION_VOIP);
    }
    #endif
}

const std::vector<std::string> AudioProcessorFactory::getAudioProcessorNames()
{
    std::vector<std::string> processorNames;
    #ifdef PROCESSOROPUS_H
    processorNames.push_back(OPUS_CODEC);
    #endif
    return processorNames;
}


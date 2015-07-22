/* 
 * File:   AudioProcessorFactory.h
 * Author: daniel
 *
 * Created on June 27, 2015, 10:15 AM
 */

#ifndef AUDIOPROCESSORFACTORY_H
#define	AUDIOPROCESSORFACTORY_H

#include "AudioProcessor.h"
#include <vector>

/*!
 * Factory-class to provide audio-processor objects without needing to know the details of the implementation.
 */
class AudioProcessorFactory
{
public:
    static const std::string OPUS_CODEC;
    static const std::string WAV_WRITER;
    static AudioProcessor* getAudioProcessor(std::string name);
    static const std::vector<std::string> getAudioProcessorNames();
private:

};

#endif	/* AUDIOPROCESSORFACTORY_H */


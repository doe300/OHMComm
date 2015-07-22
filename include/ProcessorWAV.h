/* 
 * AudioProcessor which logs the audio-communication into WAV-files
 * 
 * File:   ProcessorWAV.h
 * Author: daniel
 *
 * Created on July 22, 2015, 5:35 PM
 */

#ifndef PROCESSORWAV_H
#define	PROCESSORWAV_H

#include "AudioProcessor.h"
#include <stdio.h>
#include "wavfile.h"

class ProcessorWAV : public AudioProcessor
{
public:
    ProcessorWAV();
    virtual ~ProcessorWAV();
    
    bool configure(AudioConfiguration audioConfig);
    unsigned int getSupportedAudioFormats();
    std::vector<int> getSupportedBufferSizes(unsigned int sampleRate);
    unsigned int getSupportedSampleRates();

    unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);

    unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);
private:
    FILE* writeInputFile;
    FILE* writeOutputFile;
    
};

#endif	/* PROCESSORWAV_H */


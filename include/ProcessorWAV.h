/* 
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

/**
 * AudioProcessor which logs the audio-communication into WAV-files.
 * 
 * Both the audio-input and the audio-output can be logged separately, with the destination files
 * specified by user-input
 */
class ProcessorWAV : public AudioProcessor
{
public:
    ProcessorWAV();
    virtual ~ProcessorWAV();
    
    bool configure(AudioConfiguration audioConfig);
    
    /*!
     * The wav implementation only supports 16bit signed integer PCM samples
     */
    unsigned int getSupportedAudioFormats();
    
    /*!
     * This writer supports all/any buffer-size
     */
    std::vector<int> getSupportedBufferSizes(unsigned int sampleRate);
    
    /*!
     * The wav implementation only supports the 44.1kHz sample-rate
     */
    unsigned int getSupportedSampleRates();

    /*!
     * If input-logging is active, writes the audio-input to the input-logging file
     */
    unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);

    /*!
     * If output-logging is active, writes the audio-output to the output-logging file
     */
    unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);
private:
    FILE* writeInputFile;
    FILE* writeOutputFile;
    
};

#endif	/* PROCESSORWAV_H */


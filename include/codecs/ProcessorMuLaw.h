/* 
 * File:   ProcessorMuLaw.h
 * Author: daniel
 *
 * Created on January 8, 2016, 11:38 AM
 */

#ifndef PROCESSORMULAW_H
#define	PROCESSORMULAW_H

#include "AudioProcessor.h"
#include "g711common.h"

/**
 * Implementation of the G.711 mu-law audio-codec
 * 
 * See: https://en.wikipedia.org/wiki/%CE%9C-law_algorithm
 * See: http://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-G.711-198811-I!!PDF-E&type=items
 * 
 * Uses the Payload-type 0 (PCMU), see: https://tools.ietf.org/html/rfc3551 Section 4.5.14
 */
class ProcessorMuLaw : public AudioProcessor
{
public:
    ProcessorMuLaw(const std::string& name);
    
    ~ProcessorMuLaw();

    unsigned int getSupportedAudioFormats() const;
    
    unsigned int getSupportedSampleRates() const;
    
    const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;
    
    PayloadType getSupportedPlayloadType() const;
    
    bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode);
    
    bool cleanUp();
    
    unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
    
    unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);
    
private:
    int16_t* writeBuffer;
    uint16_t maxBufferSize;
};

#endif	/* PROCESSORMULAW_H */


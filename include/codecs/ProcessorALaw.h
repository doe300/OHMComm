/* 
 * File:   ProcessorALaw.h
 * Author: daniel
 *
 * Created on January 7, 2016, 10:56 AM
 */

#ifndef PROCESSORALAW_H
#define	PROCESSORALAW_H

#include "AudioProcessor.h"
#include "g711common.h"

/**
 * Implementation of the G.711 A-law audio-codec
 * 
 * NOTE: If not started with SIP-configuration, this will run in stereo mode, despite the RTP payload-type specifying mono.
 * The same applies to mu-law.
 * 
 * See: https://en.wikipedia.org/wiki/A-law_algorithm
 * See: http://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-G.711-198811-I!!PDF-E&type=items
 * 
 * Uses the Payload-type 8 (PCMA), see: https://tools.ietf.org/html/rfc3551 Section 4.5.14
 */
class ProcessorALaw : public AudioProcessor
{
public:
    ProcessorALaw(const std::string& name);
    
    ~ProcessorALaw();

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

#endif	/* PROCESSORALAW_H */


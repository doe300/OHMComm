/* 
 * File:   ProcessoriLBC.h
 * Author: daniel
 *
 * Created on March 2, 2016, 1:22 PM
 */
#ifdef ILBC_HEADER  //Only include of iLBC is linked
#ifndef PROCESSORILBC_H
#define	PROCESSORILBC_H

#include ILBC_HEADER
#include "AudioProcessor.h"

/*!
 * Encoder/Decoder for the iLBC-codec
 * 
 * This class wraps the default iLBC-implementation from RFC 3951 (https://tools.ietf.org/html/rfc3951)
 * 
 * RTP payload specified in: https://tools.ietf.org/html/rfc3952
 */
class ProcessoriLBC : public AudioProcessor
{
public:
    ProcessoriLBC(const std::string& name);
    virtual ~ProcessoriLBC();

    virtual unsigned int getSupportedAudioFormats() const;
    virtual unsigned int getSupportedSampleRates() const;
    virtual const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;
    virtual PayloadType getSupportedPlayloadType() const;

    virtual bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

    virtual unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);
    virtual unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);

    virtual bool cleanUp();
private:
    iLBC_encinst_t* iLBCEncoder;
    iLBC_decinst_t* iLBCDecoder;
    uint8_t frameLength;
};

#endif	/* PROCESSORILBC_H */
#endif

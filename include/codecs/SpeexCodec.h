/* 
 * File:   SpeexCodec.h
 * Author: daniel
 *
 * Created on March 2, 2016, 3:01 PM
 */
#ifdef SPEEX_HEADER //Only compile if speex is linked
#ifndef SPEEXCODEC_H
#define	SPEEXCODEC_H

#include SPEEX_HEADER
#include SPEEX_STEREO_HEADER

#include "AudioProcessor.h"
#include "processors/Resampler.h"

/*!
 * Audio-processor for the speex codec (http://speex.org)
 * 
 * RTP payload-type taken from RFC 5574 (https://tools.ietf.org/html/rfc5574)
 */
class SpeexCodec : public AudioProcessor
{
public:
    SpeexCodec(const std::string& name);
    virtual ~SpeexCodec();
    
    virtual unsigned int getSupportedAudioFormats() const;
    virtual unsigned int getSupportedSampleRates() const;
    virtual const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;
    virtual PayloadType getSupportedPlayloadType() const;

    virtual bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

    virtual unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);
    virtual unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);

    virtual bool cleanUp();
private:
    void* speexEncoder;
    void* speexDecoder;
    SpeexBits encoderBits;
    SpeexBits decoderBits;

};

#endif	/* SPEEXCODEC_H */
#endif
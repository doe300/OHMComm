#ifndef PROCESSOROPUS_H

#define PROCESSOROPUS_H
#include "AudioProcessor.h"
#include "opus.h"
#include "stdint.h"
#include "configuration.h"

class ProcessorOpus : public AudioProcessor
{
public:

	//constructor, initialises the OpusApplication type
	ProcessorOpus(std::string name, int opusApplication);

	//deliver supported Audio Formats by Opus: currently only rtaudio SINT16 is supported
	unsigned int getSupportedAudioFormats();
	//deliver supported Sample Rates by Opus: Opus supports this sampleRates: 8000, 12000, 16000, 24000, or 48000. = 1001111 binary in the format from configuration.h (every 1 stands for one supported format) = 79 decimal = 0x4F Hex
	unsigned int getSupportedSampleRates();

	//configure the Opus Processor, this creates OpusEncoder and OpusDecoderObject and initialises outputDeviceChannels and ErrorCode
	bool configure(AudioConfiguration audioConfig);

	//encodes the Data in inputBuffer(only Signed 16 bit PCM and one frame supported) and writes the encoded Data in inputBuffer 
	//supported size of one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//returns lenght of encodedPacket in Bytes
	unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);

	//decodes the Data in outputBuffer and writes it to the outputBuffer(only Signed 16 bit PCM and one frame supported)
	//supported size of one frame(2.5, 5, 10, 20, 40 or 60 ms) of audio data; at 48 kHz the permitted values are 120, 240, 480(10ms), 960(20ms), 1920, and 2880(60ms)
	//returns size of outputBuffer in Bytes
	unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

	//destructor: destroys OpusEncoder and OpusDecoderObject
	~ProcessorOpus();

private:

	OpusEncoder *OpusEncoderObject;
	OpusDecoder *OpusDecoderObject;
	int OpusApplication;
	int ErrorCode;
	//number of outputDeviceChannels needed for the calculation of outputBytes in processOutputData
	unsigned int outputDeviceChannels;

};
#endif
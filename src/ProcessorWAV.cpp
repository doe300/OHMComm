/*
 * File:   ProcessorWAV.cpp
 * Author: daniel
 *
 * Created on July 22, 2015, 5:35 PM
 */

#include "ProcessorWAV.h"
#include "UserInput.h"

using namespace wav;

ProcessorWAV::ProcessorWAV(const std::string name) : AudioProcessor(name)
{
    writeInputFile = nullptr;
    writeOutputFile = nullptr;
}

ProcessorWAV::~ProcessorWAV()
{
    if(writeInputFile != nullptr)
    {
        wavfile_close(writeInputFile);
    }
    if(writeOutputFile != nullptr)
    {
        wavfile_close(writeOutputFile);
    }
}

bool ProcessorWAV::configure(const AudioConfiguration& audioConfig)
{
    if(audioConfig.audioFormatFlag != AudioConfiguration::AUDIO_FORMAT_SINT16)
    {
        std::cerr << "Unsupported audio-format!" << std::endl;
        return false;
    }
    if(audioConfig.sampleRate != 44100)
    {
        std::cerr << "Unsupported sample-rate!" << std::endl;
        return false;
    }
    UserInput::printSection("Configure WAV-Writer");
    if(UserInput::inputBoolean("Log audio-input?"))
    {
        std::string fileName = UserInput::inputString("Type audio-input file-name");
        writeInputFile = wavfile_open(fileName.c_str());
    }
    if(UserInput::inputBoolean("Log audio-output?"))
    {
        std::string fileName = UserInput::inputString("Type audio-output file-name");
        writeOutputFile = wavfile_open(fileName.c_str());
    }
    return true;
}

unsigned int ProcessorWAV::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

const std::vector<int> ProcessorWAV::getSupportedBufferSizes(unsigned int sampleRate) const
{
    return std::vector<int>{BUFFER_SIZE_ANY};
}

unsigned int ProcessorWAV::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_44100;
}

unsigned int ProcessorWAV::processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData)
{
    if(writeInputFile != nullptr)
    {
        wavfile_write(writeInputFile, (short*)inputBuffer, userData->nBufferFrames*2);
    }
    return inputBufferByteSize;

}

unsigned int ProcessorWAV::processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData)
{
    if(writeOutputFile != nullptr)
    {
        wavfile_write(writeOutputFile, (short*)outputBuffer, userData->nBufferFrames*2);
    }
    return outputBufferByteSize;
}

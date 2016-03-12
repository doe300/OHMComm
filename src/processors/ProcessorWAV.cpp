/*
 * File:   ProcessorWAV.cpp
 * Author: daniel
 *
 * Created on July 22, 2015, 5:35 PM
 */

#include "processors/ProcessorWAV.h"

using namespace ohmcomm;
using namespace wav;

const Parameter* ProcessorWAV::INPUT_FILE_NAME = Parameters::registerParameter(Parameter(ParameterCategory::PROCESSORS, 'I', "input-wav-file", "wav-Writer. The name of the wav-file to log the audio-input", ""));
const Parameter* ProcessorWAV::OUTPUT_FILE_NAME = Parameters::registerParameter(Parameter(ParameterCategory::PROCESSORS, 'O', "output-wav-file", "wav-Writer. The name of the wav-file to log the audio-output", ""));

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

void ProcessorWAV::configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize)
{
    if(audioConfig.audioFormatFlag != AudioConfiguration::AUDIO_FORMAT_SINT16)
    {
        throw ohmcomm::configuration_error("WAV", "Unsupported audio-format!");
    }
    if(audioConfig.sampleRate != 44100)
    {
        throw ohmcomm::configuration_error("WAV", "Unsupported sample-rate!");
    }
    if(configMode->isCustomConfigurationSet(INPUT_FILE_NAME->longName, "Log audio-input?"))
    {
        std::string fileName = configMode->getCustomConfiguration(INPUT_FILE_NAME->longName, "Type audio-input file-name", "");
        writeInputFile = wavfile_open(fileName.c_str());
    }
    if(configMode->isCustomConfigurationSet(OUTPUT_FILE_NAME->longName, "Log audio-output?"))
    {
        std::string fileName = configMode->getCustomConfiguration(OUTPUT_FILE_NAME->longName, "Type audio-output file-name", "");
        writeOutputFile = wavfile_open(fileName.c_str());
    }
}

unsigned int ProcessorWAV::getSupportedAudioFormats() const
{
    return AudioConfiguration::AUDIO_FORMAT_SINT16;
}

unsigned int ProcessorWAV::getSupportedSampleRates() const
{
    return AudioConfiguration::SAMPLE_RATE_44100;
}

PayloadType ProcessorWAV::getSupportedPlayloadType() const
{
    return PayloadType::L16_2;
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

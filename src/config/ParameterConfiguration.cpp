/* 
 * File:   ParameterConfiguration.cpp
 * Author: daniel
 * 
 * Created on November 30, 2015, 5:32 PM
 */

#include "config/ParameterConfiguration.h"
#include "AudioHandlerFactory.h"
#include "RtAudio.h"

ParameterConfiguration::ParameterConfiguration(const Parameters& params) : ConfigurationMode(), params(params)
{
    //convert configuration
    //get device configuration from parameters
    int outputDeviceID = -1;
    int inputDeviceID = -1;
    if(params.isParameterSet(Parameters::AUDIO_HANDLER))
    {
        audioHandlerName = params.getParameterValue(Parameters::AUDIO_HANDLER);
    }
    else
    {
        audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    }
    if(params.isParameterSet(Parameters::OUTPUT_DEVICE))
    {
        outputDeviceID = atoi(params.getParameterValue(Parameters::OUTPUT_DEVICE).c_str());
    }
    if(params.isParameterSet(Parameters::INPUT_DEVICE))
    {
        inputDeviceID = atoi(params.getParameterValue(Parameters::INPUT_DEVICE).c_str());
    }
    useDefaultAudioConfig = !(inputDeviceID >= 0 || outputDeviceID >= 0);
    fillAudioConfiguration(outputDeviceID, inputDeviceID);
    if(params.isParameterSet(Parameters::FORCE_AUDIO_FORMAT))
    {
        useDefaultAudioConfig =false;
        audioConfig.audioFormatFlag = atoi(params.getParameterValue(Parameters::FORCE_AUDIO_FORMAT).c_str());
        audioConfig.forceAudioFormatFlag = atoi(params.getParameterValue(Parameters::FORCE_AUDIO_FORMAT).c_str());
    }
    if(params.isParameterSet(Parameters::FORCE_SAMPLE_RATE))
    {
        useDefaultAudioConfig =false;
        audioConfig.sampleRate = atoi(params.getParameterValue(Parameters::FORCE_SAMPLE_RATE).c_str());
        audioConfig.forceSampleRate = atoi(params.getParameterValue(Parameters::FORCE_SAMPLE_RATE).c_str());
    }

    //get network configuration from parameters
    networkConfig.remoteIPAddress = params.getParameterValue(Parameters::REMOTE_ADDRESS);
    networkConfig.localPort = atoi(params.getParameterValue(Parameters::LOCAL_PORT).c_str());
    networkConfig.remotePort = atoi(params.getParameterValue(Parameters::REMOTE_PORT).c_str());

    //get audio-processors from parameters
    processorNames.reserve(params.getAudioProcessors().size());
    for(const std::string& procName : params.getAudioProcessors())
    {
        processorNames.push_back(procName);
    }
    profileProcessors = params.isParameterSet(Parameters::PROFILE_PROCESSORS);
    logToFile = params.isParameterSet(Parameters::LOG_TO_FILE);
    logFileName = params.getParameterValue(Parameters::LOG_TO_FILE);

    waitForConfigurationRequest = params.isParameterSet(Parameters::WAIT_FOR_PASSIVE_CONFIG);
    //we completely configured OHMComm
    isConfigurationDone = true;
}

bool ParameterConfiguration::runConfiguration()
{
    return isConfigurationDone;
}

void ParameterConfiguration::fillAudioConfiguration(int outputDeviceID, int inputDeviceID)
{
    RtAudio audioDevices;
    if(outputDeviceID < 0)
    {
        outputDeviceID = audioDevices.getDefaultOutputDevice();
    }
    if(inputDeviceID < 0)
    {
        inputDeviceID = audioDevices.getDefaultInputDevice();
    }
    //we always use stereo
    audioConfig.outputDeviceChannels = 2;
    audioConfig.inputDeviceChannels = 2;

    audioConfig.outputDeviceID = outputDeviceID;
    audioConfig.inputDeviceID = inputDeviceID;
}

const std::string ParameterConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    const Parameter* param = Parameters::getParameter(key);
    if(param != nullptr)
    {
        return params.getParameterValue(param);
    }
    return defaultValue;
}

int ParameterConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    const Parameter* param = Parameters::getParameter(key);
    if(param != nullptr)
    {
        return atoi(params.getParameterValue(param).data());
    }
    return defaultValue;
}

bool ParameterConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    const Parameter* param = Parameters::getParameter(key);
    if(param != nullptr)
    {
        return atoi(params.getParameterValue(param).data());
    }
    return defaultValue;
}

bool ParameterConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    const Parameter* param = Parameters::getParameter(key);
    if(param != nullptr)
    {
        return params.isParameterSet(param);
    }
    return false;
}
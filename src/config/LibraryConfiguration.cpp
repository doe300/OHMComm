/* 
 * File:   LibraryConfiguration.cpp
 * Author: daniel
 * 
 * Created on November 30, 2015, 5:34 PM
 */

#include "config/LibraryConfiguration.h"
#include "AudioHandlerFactory.h"

LibraryConfiguration::LibraryConfiguration()
{
    //initialize configuration with default values as far as possible
    audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    createDefaultNetworkConfiguration();
    //XXX we should move away from using loopback as valid configuration
    isNetworkConfigured = true;
}

bool LibraryConfiguration::runConfiguration()
{
    return isConfigured();
}

bool LibraryConfiguration::isConfigured() const
{
    return isAudioConfigured && isNetworkConfigured && isProcessorsConfigured;
}

const std::string LibraryConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return customConfig.at(key);
}

int LibraryConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return atoi(customConfig.at(key).data());
}

bool LibraryConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return atoi(customConfig.at(key).data());
}

bool LibraryConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return customConfig.find(key) != customConfig.end();
}

void LibraryConfiguration::configureAudio(const std::string audioHandlerName, const AudioConfiguration* audioConfig)
{
    this->audioHandlerName = audioHandlerName;
    if(audioConfig != nullptr)
    {
        this->audioConfig = *audioConfig;
        useDefaultAudioConfig = false;
    }
    isAudioConfigured = true;
}

void LibraryConfiguration::configureNetwork(const NetworkConfiguration& networkConfig)
{
    this->networkConfig = networkConfig;
    isNetworkConfigured = true;
}

void LibraryConfiguration::configureProcessors(const std::vector<std::string>& processorNames, bool profileProcessors)
{
    this->profileProcessors = profileProcessors;
    this->processorNames.reserve(processorNames.size());
    for(const std::string& name : processorNames)
    {
        this->processorNames.push_back(name);
    }
    this->profileProcessors = profileProcessors;

    isProcessorsConfigured = true;
}

void LibraryConfiguration::configureLogToFile(const std::string logFileName)
{
    logToFile = true;
    this->logFileName = logFileName;
}

void LibraryConfiguration::configureWaitForConfigurationRequest(bool waitForConfig)
{
    waitForConfigurationRequest = waitForConfig;
}

void LibraryConfiguration::configureCustomValue(std::string key, std::string value)
{
    customConfig[key] = value;
}

void LibraryConfiguration::configureCustomValue(std::string key, int value)
{
    customConfig[key] = std::to_string(value);
}

void LibraryConfiguration::configureCustomValue(std::string key, bool value)
{
    customConfig[key] = std::to_string(value ? 1 : 0);
}
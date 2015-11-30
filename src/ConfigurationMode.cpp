/*
 * File:   ConfigurationMode.cpp
 * Author: daniel
 *
 * Created on August 19, 2015, 4:48 PM
 */
#include "ConfigurationMode.h"

ConfigurationMode::ConfigurationMode() : audioConfig( {0} ), networkConfig( {0} ), processorNames(std::vector<std::string>(0))
{
}

ConfigurationMode::~ConfigurationMode()
{
}

const std::pair<std::string, bool> ConfigurationMode::getAudioHandlerConfiguration() const
{
    if(!isConfigured())
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return std::make_pair(audioHandlerName, !useDefaultAudioConfig);
}


const AudioConfiguration ConfigurationMode::getAudioConfiguration() const
{
    if(!isConfigured())
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return audioConfig;
}

const NetworkConfiguration ConfigurationMode::getNetworkConfiguration() const
{
    if(!isConfigured())
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return networkConfig;
}

const NetworkConfiguration ConfigurationMode::getRTCPNetworkConfiguration() const
{
    if(!isConfigured())
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    NetworkConfiguration rtcpConfig;
    rtcpConfig.localPort = networkConfig.localPort+1;
    rtcpConfig.remoteIPAddress = networkConfig.remoteIPAddress;
    rtcpConfig.remotePort = networkConfig.remotePort+1;
    return rtcpConfig;
}

bool ConfigurationMode::isConfigured() const
{
    return isConfigurationDone;
}

bool ConfigurationMode::getAudioProcessorsConfiguration(std::vector<std::string>& processorNames) const
{
    if(!isConfigured())
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    processorNames.reserve(this->processorNames.size());
    for(const std::string& procName : this->processorNames)
    {
        processorNames.push_back(procName);
    }
    return profileProcessors;
}

const std::pair<bool, std::string> ConfigurationMode::getLogToFileConfiguration() const
{
    if(!isConfigured())
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return std::pair<bool, std::string>(logToFile, logFileName);
}

const bool ConfigurationMode::isWaitForConfigurationRequest() const
{
    return waitForConfigurationRequest;
}

void ConfigurationMode::updateAudioConfiguration(const AudioConfiguration& audioConfig)
{
    this->audioConfig = audioConfig;
}

void ConfigurationMode::createDefaultNetworkConfiguration()
{
    networkConfig.remoteIPAddress = "127.0.0.1";
    networkConfig.localPort = DEFAULT_NETWORK_PORT;
    networkConfig.remotePort = DEFAULT_NETWORK_PORT;
}
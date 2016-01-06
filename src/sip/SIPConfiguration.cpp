/* 
 * File:   SIPConfiguration.cpp
 * Author: daniel
 * 
 * Created on December 2, 2015, 1:06 PM
 */

#include "sip/SIPConfiguration.h"
#include "AudioHandlerFactory.h"

#include <chrono>

SIPConfiguration::SIPConfiguration(const NetworkConfiguration& sipConfig, bool profileProcessors, const std::string& logFile) : 
    ConfigurationMode(), handler(sipConfig, "remote", [this](const MediaDescription media, const NetworkConfiguration rtpConfig, const NetworkConfiguration rtcpConfig){this->setConfig(media, rtpConfig, rtcpConfig);}), rtcpConfig({0})
{
    useDefaultAudioConfig = false;
    audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    networkConfig.localPort = DEFAULT_NETWORK_PORT;
    networkConfig.remoteIPAddress = sipConfig.remoteIPAddress;
    profileProcessors = profileProcessors;
    waitForConfigurationRequest = false;
    if(!logFile.empty())
    {
        logToFile = true;
        logFileName = logFile;
    }
    else
    {
        logToFile = false;
    }
}

SIPConfiguration::~SIPConfiguration()
{
    handler.shutdown();
}

bool SIPConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }
    handler.startUp();
    
    int timeLeft = SIPConfiguration::MAX_WAIT_TIME;
    
    //wait for configuration to be done
    while(!isConfigurationDone && handler.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // maximum time to wait before aborting configuration
        timeLeft -= 100;
        //FIXME can currently not be canceled by user input
        if(timeLeft <= 0)
        {
            //when aborting, let SIPHandler send CANCEL
            handler.shutdown();
            break;
        }
    }
    return isConfigurationDone;
}

const NetworkConfiguration SIPConfiguration::getRTCPNetworkConfiguration() const
{
    return rtcpConfig;
}


const std::string SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    //TODO how to support custom configuration??
    return defaultValue;
}

const int SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    return defaultValue;
}

const bool SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    return defaultValue;
}

const bool SIPConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return false;
}

void SIPConfiguration::onRegister(PlaybackObservee* ohmComm)
{
    handler.stopCallback = ohmComm->createStopCallback();
}

void SIPConfiguration::onPlaybackStop()
{
    handler.shutdown();
}

void SIPConfiguration::setConfig(const MediaDescription& media, const NetworkConfiguration& rtpConfig, const NetworkConfiguration& customRTCPConfig)
{
    //RTP-config
    networkConfig.localPort = rtpConfig.localPort;
    networkConfig.remoteIPAddress = rtpConfig.remoteIPAddress;
    networkConfig.remotePort = media.port;
    //RTCP-config
    //allows for custom remote RTCP port (and address), see RFC 3605
    rtcpConfig.localPort = networkConfig.localPort + 1;
    rtcpConfig.remotePort = networkConfig.remotePort + 1;
    rtcpConfig.remoteIPAddress = networkConfig.remoteIPAddress;
    if(customRTCPConfig.remotePort != 0)
    {
        rtcpConfig.remotePort = customRTCPConfig.remotePort;
    }
    if(!customRTCPConfig.remoteIPAddress.empty())
    {
        rtcpConfig.remoteIPAddress = customRTCPConfig.remoteIPAddress;
    }
    
    //audio-config
    audioConfig.forceSampleRate = media.sampleRate;
    audioConfig.inputDeviceChannels = media.numChannels;
    audioConfig.outputDeviceChannels = media.numChannels;
    payloadType = media.payloadType;
    SupportedFormat format = media.getFormat();
    if(!format.processorName.empty())
    {
        //format "PCM" has no processor name set, do don't add an empty one
        processorNames.push_back(format.processorName);
    }
    
    isConfigurationDone = true;
}
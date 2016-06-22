/* 
 * File:   SIPConfiguration.cpp
 * Author: daniel
 * 
 * Created on December 2, 2015, 1:06 PM
 */

#include "Logger.h"
#include "sip/SIPConfiguration.h"
#include "processors/AudioProcessorFactory.h"
#include "Parameters.h"

#include <chrono>

using namespace ohmcomm::sip;

SIPConfiguration::SIPConfiguration(const ohmcomm::Parameters& params, const ohmcomm::NetworkConfiguration& sipConfig) : 
    ParameterConfiguration(params), handler(sipConfig, params.getParameterValue(Parameters::SIP_REMOTE_USER), [this](const MediaDescription media, const ohmcomm::NetworkConfiguration rtpConfig, const ohmcomm::NetworkConfiguration rtcpConfig){this->setConfig(media, rtpConfig, rtcpConfig);}, params.getParameterValue(Parameters::SIP_REGISTER_USER), params.getParameterValue(Parameters::SIP_REGISTER_PASSWORD)), rtcpConfig({0})
{
    //overwrite settings from ParameterConfiguration
    useDefaultAudioConfig = false;
    networkConfig.localPort = DEFAULT_NETWORK_PORT;
    networkConfig.remoteIPAddress = sipConfig.remoteIPAddress;
    isConfigurationDone = false;
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
    
    //wait a bit for handler to have started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // maximum time to wait before aborting configuration
    int timeLeft = SIPConfiguration::MAX_WAIT_TIME;
    ohmcomm::info("SIP") << "Establishing connection ... " << ohmcomm::endl;
    ohmcomm::info("SIP") << "Press Enter to cancel" << ohmcomm::endl;
    
    //wait for configuration to be done
    while(!isConfigurationDone && handler.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(!handler.authentication || !handler.authentication->isAuthenticated())
        {
            //only time out if this UAC is not registered with an UAS
            timeLeft -= 100;
        }
        //abort configuration on timeout or user input
        if(timeLeft <= 0 || Utility::waitForUserInput(10) > 0)
        {
            //when aborting, let SIPHandler send CANCEL
            handler.shutdown();
            break;
        }
    }
    return isConfigurationDone;
}

const ohmcomm::NetworkConfiguration SIPConfiguration::getRTCPNetworkConfiguration() const
{
    return rtcpConfig;
}


const std::string SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    if(customConfig.count(key) != 0)
        return customConfig.at(key);
    return ParameterConfiguration::getCustomConfiguration(key, message, defaultValue);
}

int SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    if(customConfig.count(key) != 0)
        return atoi(customConfig.at(key).data());
    return ParameterConfiguration::getCustomConfiguration(key, message, defaultValue);
}

bool SIPConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    if(customConfig.count(key) != 0)
        return atoi(customConfig.at(key).data());
    return ParameterConfiguration::getCustomConfiguration(key, message, defaultValue);
}

bool SIPConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return customConfig.count(key) != 0 || ParameterConfiguration::isCustomConfigurationSet(key, message);
}

void SIPConfiguration::onRegister(ohmcomm::PlaybackObservee* ohmComm)
{
    //TODO remove stop-callback, make participant-leave
    handler.stopCallback = ohmComm->createStopCallback();
}

void SIPConfiguration::onPlaybackStop()
{
    handler.shutdown();
}

void SIPConfiguration::setConfig(const MediaDescription& media, const ohmcomm::NetworkConfiguration& rtpConfig, const ohmcomm::NetworkConfiguration& customRTCPConfig)
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
    for(const KeyValuePair<std::string>& param : media.formatParams.fields)
    {
        if(SupportedFormat::FORMAT_OPUS_DTX.compare(param.key) == 0 && param.value.compare("1") == 0)
        {
            //if the other side supports DTX, we do too
            //add "Gain Control" to calculate the silence-threshold
            processorNames.push_back(AudioProcessorFactory::GAIN_CONTROL);
            //set enable-DTX parameter for RTP-processor to use DTX
            customConfig[Parameters::ENABLE_DTX->longName] = "1";
        }
    }
    if(!format.processorName.empty())
    {
        //format "PCM" has no processor name set, do don't add an empty one
        processorNames.push_back(format.processorName);
    }
    
    isConfigurationDone = true;
}
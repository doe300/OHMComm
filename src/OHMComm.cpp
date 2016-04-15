/*
 * File:   OHMComm.cpp
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#include "OHMComm.h"
#include "Logger.h"
#include "rtp/ProcessorRTP.h"
#include "processors/AudioProcessorFactory.h"
#include "audio/AudioHandlerFactory.h"
#include "config/ConfigurationMode.h"

using namespace ohmcomm;

OHMComm::OHMComm(ConfigurationMode* mode) : configurationMode(mode), audioHandler(nullptr)
{
    registerPlaybackListener(configurationMode);
    rtp::ParticipantDatabase::registerListener(*this);
}

OHMComm::~OHMComm()
{
    rtp::ParticipantDatabase::unregisterListener(*this);
}

std::shared_ptr<ConfigurationMode> OHMComm::getConfigurationMode() const
{
    return configurationMode;
}

bool OHMComm::isConfigurationActive() const
{
    return configurationActive;
}

bool OHMComm::isConfigurationDone(bool runConfiguration)
{
    if(!configurationActive)
    {
        return true;
    }
    if(configurationMode->isConfigured())
    {
        return true;
    }
    if(runConfiguration)
    {
        //run interactive configuration
        return configurationMode->runConfiguration();
    }
    return false;
}

void OHMComm::startAudioThreads()
{
    if(running)
    {
        ohmcomm::info("OHMComm") << "OHMComm already running..." << ohmcomm::endl;
        return;
    }
    if(!isConfigurationDone(false))
    {
        throw std::runtime_error("OHMComm not fully configured!");
    }
    configurationActive = false;
    //initialize audio-objects
    if(configurationMode->getAudioHandlerConfiguration().second)
    {
        audioHandler = AudioHandlerFactory::getAudioHandler(configurationMode->getAudioHandlerConfiguration().first, configurationMode->getAudioConfiguration());
    }
    else
    {
        audioHandler = AudioHandlerFactory::getAudioHandler(configurationMode->getAudioHandlerConfiguration().first);
    }
    std::vector<std::string> procNames(0);
    bool profileProcessors = configurationMode->getAudioProcessorsConfiguration(procNames);
    PayloadType payloadType = PayloadType::L16_2;
    for(const std::string& procName : procNames)
    {
        AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(procName, profileProcessors);
        audioHandler->getProcessors().addProcessor(proc);
        //only the last non-default payload-type is required
        payloadType = proc->getSupportedPlayloadType() == PayloadType::ALL ? payloadType : proc->getSupportedPlayloadType();
    }
    if(configurationMode->getPayloadType() != PayloadType::ALL)
    {
        //if we use a custom payload-type (e.g. for SIP-config), it must be set here
        payloadType = (PayloadType)configurationMode->getPayloadType();
    }
    configureRTPProcessor(profileProcessors, payloadType);

    //run threads

    if(!audioHandler->prepare(configurationMode))
    {
        throw std::runtime_error("Failed to configure audio-handler!");
    }
    //write configuration back to ConfigurationMode
    configurationMode->updateAudioConfiguration(audioHandler->getAudioConfiguration());

    //begin audio-playback
    startAudio();
}

void OHMComm::startAudio()
{
    notifyPlaybackStart();
    if((audioHandler->getMode() & PlaybackMode::DUPLEX) == PlaybackMode::DUPLEX)
    {
        ohmcomm::info("OHMComm") << "starting duplex mode ..." << ohmcomm::endl;
        audioHandler->start(PlaybackMode::DUPLEX);
    }
    else if((audioHandler->getMode() & PlaybackMode::OUTPUT) == PlaybackMode::OUTPUT)
    {
        ohmcomm::info("OHMComm") << "starting playback mode ..." << ohmcomm::endl;
        audioHandler->start(PlaybackMode::OUTPUT);
    }
    else if((audioHandler->getMode() & PlaybackMode::INPUT) == PlaybackMode::INPUT)
    {
        ohmcomm::info("OHMComm") << "starting recording mode ..." << ohmcomm::endl;
        audioHandler->start(PlaybackMode::INPUT);
    }
    else
    {
        ohmcomm::error("OHMComm") << "No supported playback-mode configured!" << ohmcomm::endl;
        return;
    }
    
    ohmcomm::info("OHMComm") << "OHMComm started!" << ohmcomm::endl;
    running = true;
}

void OHMComm::stopAudioThreads()
{
    if(!running)
    {
        //to prevent recursive calls
        ohmcomm::info("OHMComm") << "OHMComm is not running" << ohmcomm::endl;
        return;
    }
    running = false;

    audioHandler->stop();
    notifyPlaybackStop();
    
    //give threads some time to shut down
    //this is mostly to prevent from parallel logging to console
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    ohmcomm::info("OHMComm") << "OHMComm stopped!" << ohmcomm::endl;
    const std::pair<bool, std::string> logConfig = configurationMode->getLogToFileConfiguration();
    if(logConfig.first)
    {
        Statistics::printStatisticsToFile(logConfig.second);
        ohmcomm::info("OHMComm") << "Statistics file written" << ohmcomm::endl;
    }
    //print statistics to stdout anyway
    Statistics::printStatistics();
    
    //if this method was called from one of the background-tasks, only the main-thread (waiting for input) remains.
    std::cout << "Type Enter to exit..." << std::endl;
}

bool OHMComm::isRunning() const
{
    return running;
}

void OHMComm::onRemoteRemoved(const unsigned int ssrc)
{
    if(rtp::ParticipantDatabase::getNumberOfRemotes() == 0)
    {
        ohmcomm::info("OMHComm") << "No more communication partners, shutting down..." << ohmcomm::endl;
        stopAudioThreads();
    }
}

void OHMComm::configureRTPProcessor(bool profileProcessors, const PayloadType payloadType)
{
    rtp::ProcessorRTP* rtpProcessor = new rtp::ProcessorRTP("RTP-Processor", configurationMode->getNetworkConfiguration(), payloadType);
    if(profileProcessors)
    {
        //enabled profiling of the RTP processor
        audioHandler->getProcessors().addProcessor(new ProfilingAudioProcessor(rtpProcessor));
    }
    else
    {
        audioHandler->getProcessors().addProcessor(rtpProcessor);
    }
}

std::function<void ()> OHMComm::createStopCallback()
{
    return std::bind(&OHMComm::stopAudioThreads,this);
}
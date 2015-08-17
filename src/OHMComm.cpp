/* 
 * File:   OHMComm.cpp
 * Author: daniel
 * 
 * Created on August 17, 2015, 4:42 PM
 */

#include "OHMComm.h"
#include "UDPWrapper.h"
#include "ProcessorRTP.h"

OHMComm::OHMComm(const ConfigurationMode mode)
    : rtpBuffer(new RTPBuffer(256, 1000)), configurationMode(mode), audioHandler(nullptr), networkWrapper(nullptr), listener(nullptr)
{
    if(mode == ConfigurationMode::PARAMETERIZED)
    {
        throw std::invalid_argument("Parameterized mode can only be initialized by passing Parameters!");
    }
    if(mode == ConfigurationMode::PROGRAMMATICALLY)
    {
        //initialize configuration with default values as far as possible

        //we set default audio-handler
        isAudioConfigured = true;
        audioHandler = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::getDefaultAudioHandlerName());
        
        isNetworkConfigured = true;
        std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(createDefaultNetworkConfiguration()));
        networkWrapper = std::move(tmp);
        
        //we can't initialize audio-processors
        isProcessorsConfigured = false;
    }
    if(mode == ConfigurationMode::INTERACTIVE)
    {
        //all configuration has to be done interactively
        isAudioConfigured = false;
        isNetworkConfigured = false;
        isProcessorsConfigured = false;
    }
}

OHMComm::OHMComm(const Parameters params)
    : rtpBuffer(new RTPBuffer(256, 1000)), configurationMode(ConfigurationMode::PARAMETERIZED), audioHandler(nullptr), networkWrapper(nullptr), listener(nullptr)
{
    //convert configuration
    //get device configuration from parameters
    int outputDeviceID = -1;
    int inputDeviceID = -1;
    if(params.isParameterSet(Parameters::OUTPUT_DEVICE))
    {
        outputDeviceID = atoi(params.getParameterValue(Parameters::OUTPUT_DEVICE).c_str());
    }
    if(params.isParameterSet(Parameters::INPUT_DEVICE))
    {
        inputDeviceID = atoi(params.getParameterValue(Parameters::INPUT_DEVICE).c_str());
    }
    AudioConfiguration audioConfig = fillAudioConfiguration(outputDeviceID, inputDeviceID);
    isAudioConfigured = true;
    audioHandler = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::getDefaultAudioHandlerName(), audioConfig);
    
    //get network configuration from parameters
    isNetworkConfigured = true;
    NetworkConfiguration networkConfig{0};
    networkConfig.addressOutgoing = params.getParameterValue(Parameters::REMOTE_ADDRESS);
    networkConfig.portIncoming = atoi(params.getParameterValue(Parameters::LOCAL_PORT).c_str());
    networkConfig.portOutgoing = atoi(params.getParameterValue(Parameters::REMOTE_PORT).c_str());
    std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(networkConfig));
    networkWrapper = std::move(tmp);
    
    //get audio-processors from parameters
    isProcessorsConfigured = true;
    profileProcessors = params.isParameterSet(Parameters::PROFILE_PROCESSORS);
    logStatisticsToFile = params.isParameterSet(Parameters::LOG_TO_FILE);
    logFileName = params.getParameterValue(Parameters::LOG_TO_FILE);
    for(const std::string& procName : params.getAudioProcessors())
    {
        AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(procName, profileProcessors);
        if(proc != nullptr)
        {
            audioHandler->addProcessor(proc);
        }
    }
    configureRTPProcessor();
}

OHMComm::~OHMComm()
{
    audioHandler.reset(nullptr);
    listener.reset(nullptr);
    networkWrapper.reset();
    rtpBuffer.reset();
}

const ConfigurationMode OHMComm::getConfigurationMode()
{
    return configurationMode;
}

const bool OHMComm::isConfigurationActive()
{
    return configurationActive;
}

bool OHMComm::isConfigurationDone(bool runInteractiveConfiguration)
{
    if(!configurationActive)
    {
        return true;
    }
    if(isAudioConfigured && isNetworkConfigured && isProcessorsConfigured)
    {
        return true;
    }
    if(runInteractiveConfiguration && configurationMode == ConfigurationMode::INTERACTIVE)
    {
        //run interactive configuration
        configureInteractive();
    }
    return isAudioConfigured && isNetworkConfigured && isProcessorsConfigured;
}

void OHMComm::configureInteractive()
{
    if(!configurationActive || configurationMode != ConfigurationMode::INTERACTIVE)
    {
        return;
    }
    if(!isAudioConfigured)
    {
        //interactive audio configuration
        if (!UserInput::inputBoolean("Load default audio config?"))
        {
            const std::vector<std::string> audioHandlers = AudioHandlerFactory::getAudioHandlerNames();
            //manually configure audio-handler, so manually select it
            std::string selectedAudioHander;
            if(audioHandlers.size() > 1)
            {
                //only let user choose if there is more than 1 audio-handler
                selectedAudioHander = UserInput::selectOption("Select audio handler", audioHandlers, AudioHandlerFactory::getDefaultAudioHandlerName());
            }
            else 
            {
                selectedAudioHander = *audioHandlers.begin();
            }
            std::cout << "Using AudioHandler: " << selectedAudioHander << std::endl;
            AudioConfiguration audioConfig = interactivelyConfigureAudioDevices();
            audioHandler = AudioHandlerFactory::getAudioHandler(selectedAudioHander, audioConfig);
        }
        else
        {
            //use RtAudioWrapper as default implementation
            audioHandler = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::getDefaultAudioHandlerName());
        }
        isAudioConfigured = true;
    }
    if(!isNetworkConfigured)
    {
        //interactive network configuration
        if (!UserInput::inputBoolean("Load default network config?"))
        {
            NetworkConfiguration networkConfig = interactivelyConfigureNetwork();
            std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(networkConfig));
            networkWrapper = std::move(tmp);
        }
        else
        {
            std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(createDefaultNetworkConfiguration()));
            networkWrapper = std::move(tmp);
        }
        isNetworkConfigured = true;
            
    }
    if(!isProcessorsConfigured)
    {
        interactivelyConfigureProcessors();
        isProcessorsConfigured = true;
    }
}

void OHMComm::startAudioThreads()
{
    if(!isConfigurationDone(false))
    {
        throw std::runtime_error("OHMComm not fully configured!");
    }
    configurationActive = false;
    if(!audioHandler->prepare())
    {
        throw std::runtime_error("Failed to configure audio-handler!");
    }
    listener.reset(new RTPListener(networkWrapper, rtpBuffer, audioHandler->getBufferSize(), this));
    
    listener->startUp();
    audioHandler->startDuplexMode();
    
    running = true;
}

void OHMComm::stopAudioThreads()
{
    running = false;
    
    audioHandler->stop();
    listener->shutdown();
    networkWrapper->closeNetwork();
    
    if(logStatisticsToFile)
    {
        Statistics::printStatisticsToFile(logFileName);
    }
    //print statistics to stdout anyway
    Statistics::printStatistics();
}

bool OHMComm::isRunning()
{
    return running;
}

void OHMComm::configureAudio(const std::string audioHandlerName, const AudioConfiguration& audioConfig)
{
    checkConfigurable();
    audioHandler = AudioHandlerFactory::getAudioHandler(audioHandlerName, audioConfig);
    isAudioConfigured = true;
}

void OHMComm::configureNetwork(const NetworkConfiguration& networkConfig)
{
    checkConfigurable();
    std::unique_ptr<NetworkWrapper> tmp(new UDPWrapper(networkConfig));
    networkWrapper = std::move(tmp);
}

void OHMComm::configureProcessors(const std::vector<std::string>& processorNames, bool profileProcessors)
{
    checkConfigurable();
    if(!isAudioConfigured)
    {
        throw std::runtime_error("AudioHandler needs to be configured before the AudioProcessors!");
    }
    this->profileProcessors = profileProcessors;
    for(const std::string& procName : processorNames)
    {
        AudioProcessor* proc = AudioProcessorFactory::getAudioProcessor(procName, profileProcessors);
        if(proc != nullptr)
        {
            audioHandler->addProcessor(proc);
        }
    }
    configureRTPProcessor();
}

void OHMComm::configureLogToFile(const std::string logFileName)
{
    logStatisticsToFile = true;
    this->logFileName = logFileName;
}

////
//  Private helper methods
////

void OHMComm::checkConfigurable()
{
    if(!configurationActive)
    {
        throw std::runtime_error("Can't configure after OHMComm has been started!");
    }
    if(configurationMode != ConfigurationMode::PROGRAMMATICALLY)
    {
        throw std::runtime_error("Can't programmatically configure a pre-configured OHMComm!");
    }
}

AudioConfiguration OHMComm::fillAudioConfiguration(int outputDeviceID, int inputDeviceID)
{
    AudioConfiguration audioConfig;
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
    audioConfig.outputDeviceName = audioDevices.getDeviceInfo(outputDeviceID).name;
    
    audioConfig.inputDeviceID = inputDeviceID;
    audioConfig.inputDeviceName = audioDevices.getDeviceInfo(inputDeviceID).name;
    
    return audioConfig;
}

AudioConfiguration OHMComm::interactivelyConfigureAudioDevices()
{
    AudioConfiguration audioConfig;
    UserInput::printSection("Audio configuration");

    RtAudio AudioDevices;
    // Determine the number of available Audio Devices 
    unsigned int availableAudioDevices = AudioDevices.getDeviceCount();
    std::vector<std::string> audioDeviceNames(availableAudioDevices);

    std::cout << "Available Audio Devices: " << std::endl;
    std::cout << std::endl;
    // printing available Audio Devices
    RtAudio::DeviceInfo DeviceInfo;

    unsigned int DefaultOutputDeviceID = 0;
    unsigned int DefaultInputDeviceID = 0;

    for (unsigned int i = 0 ; i < availableAudioDevices ; i++)
    {
        DeviceInfo = AudioDevices.getDeviceInfo(i);
        if (DeviceInfo.probed == true) //Audio Device successfully probed
        {
            std::cout << "Device ID: " << i << std::endl;
            std::cout << "Device Name = " << DeviceInfo.name << std::endl;
            std::cout << "Maximum output channels = " << DeviceInfo.outputChannels << std::endl;
            std::cout << "Maximum input channels = " << DeviceInfo.inputChannels << std::endl;
            std::cout << "Maximum duplex channels = " << DeviceInfo.duplexChannels << std::endl;
            std::cout << "Default output: ";
            if (DeviceInfo.isDefaultOutput == true)
            {
                std::cout << "Yes" << std::endl;
                DefaultOutputDeviceID = i;

            }
            else
            {
                std::cout << "No" << std::endl;
            }
            std::cout << "Default input: ";
            if (DeviceInfo.isDefaultInput == true)
            {
                std::cout << "Yes" << std::endl;
                DefaultInputDeviceID = i;
            }
            else
            {
                std::cout << "No" << std::endl;
            }
            std::cout << "Supported Sample Rates: ";
            for (unsigned int sampleRate : DeviceInfo.sampleRates)
            {
                std::cout << sampleRate << " ";
            }
            std::cout << std::endl;
            std::cout << "Native Audio Formats Flag = " << DeviceInfo.nativeFormats << std::endl;
            std::cout << std::endl;
            
            audioDeviceNames[i] = (DeviceInfo.name + (DeviceInfo.isDefaultInput && DeviceInfo.isDefaultOutput ? " (default in/out)" : 
                (DeviceInfo.isDefaultInput ? " (default in)" : (DeviceInfo.isDefaultOutput ? " (default out)" : ""))));
        }
    }

    //Choose output Device
    unsigned int OutputDeviceID = UserInput::selectOptionIndex("Choose output Device ID", audioDeviceNames, DefaultOutputDeviceID);

    RtAudio::DeviceInfo OutputDeviceInfo = AudioDevices.getDeviceInfo(OutputDeviceID);

    //Configure ID of the Output Audio Device
    audioConfig.outputDeviceID = OutputDeviceID;
    std::cout << "-> Using output Device ID: " << audioConfig.outputDeviceID << std::endl;

    //Configure Name of the Output Audio Device
    audioConfig.outputDeviceName = OutputDeviceInfo.name;
    std::cout << "-> Using output Device Name: " << audioConfig.outputDeviceName << std::endl;

    //Configure outputDeviceChannels (we always use stereo)
    audioConfig.outputDeviceChannels = 2;

    //sample-rate, audio-format and buffer-size is defined by the enabled Processors so we can skip them

    //Choose input Device
    unsigned int InputDeviceID = UserInput::selectOptionIndex("Choose input Device ID", audioDeviceNames, DefaultInputDeviceID);

    RtAudio::DeviceInfo InputDeviceInfo = AudioDevices.getDeviceInfo(InputDeviceID);

    //Configure ID of the Input Audio Device
    audioConfig.inputDeviceID = InputDeviceID;
    std::cout << "-> Using input Device ID " << audioConfig.inputDeviceID << std::endl;

    //Configure Name of the Input Audio Device
    audioConfig.inputDeviceName = InputDeviceInfo.name;
    std::cout << "-> Using input Device Name: " << audioConfig.inputDeviceName << std::endl;

    //Configure inputDeviceChannels (we always use stereo)
    audioConfig.inputDeviceChannels = 2;
    
    return audioConfig;
}

NetworkConfiguration OHMComm::interactivelyConfigureNetwork()
{
    NetworkConfiguration networkConfiguration{0};
    UserInput::printSection("Network configuration");
    
    //1. remote address
    std::string ipString = UserInput::inputString("1. Input destination IP address");
    networkConfiguration.addressOutgoing = ipString;
    
    //2. remote and local ports
    int destPort = UserInput::inputNumber("2. Input destination port", false, false);
    int localPort = UserInput::inputNumber("3. Input local port", false, false);
    networkConfiguration.portOutgoing = destPort;
    networkConfiguration.portIncoming = localPort;

    std::cout << "Network configuration set." << std::endl;
    
    return networkConfiguration;
}

void OHMComm::interactivelyConfigureProcessors()
{
    std::vector<std::string> audioProcessors = AudioProcessorFactory::getAudioProcessorNames();
    //interactive processor configuration
    profileProcessors = UserInput::inputBoolean("Create profiler for audio-processors?");
    logStatisticsToFile = UserInput::inputBoolean("Log statistics and profiling-results to file?");
    if(logStatisticsToFile)
    {
        logFileName = UserInput::inputString("Type log-file name");
    }
    // add processors to the process chain
    audioProcessors.push_back("End");
    unsigned int selectedIndex;
    std::cout << "The AudioProcessors should be added in the order they are used on the sending side!" << std::endl;
    while((selectedIndex = UserInput::selectOptionIndex("Select next AudioProcessor to add", audioProcessors, audioProcessors.size()-1)) != audioProcessors.size()-1)
    {
        audioHandler->addProcessor(AudioProcessorFactory::getAudioProcessor(audioProcessors.at(selectedIndex), profileProcessors));
        audioProcessors.at(selectedIndex) = audioProcessors.at(selectedIndex) + " (added)";
    }
    configureRTPProcessor();
}

void OHMComm::configureRTPProcessor()
{
    ProcessorRTP* rtpProcessor = new ProcessorRTP("RTP-Processor", networkWrapper, rtpBuffer);
    if(profileProcessors)
    {
        //enabled profiling of the RTP processor
        audioHandler->addProcessor(new ProfilingAudioProcessor(rtpProcessor));
    }
    else
    {
        audioHandler->addProcessor(rtpProcessor);
    }
}

NetworkConfiguration OHMComm::createDefaultNetworkConfiguration()
{
    NetworkConfiguration networkConfig{0};
    networkConfig.addressOutgoing = "127.0.0.1";
    networkConfig.portIncoming = DEFAULT_NETWORK_PORT;
    networkConfig.portOutgoing = DEFAULT_NETWORK_PORT;
    return networkConfig;
}

int main(int argc, char* argv[])
{
    ////
    // Configuration
    ////
    
    OHMComm* ohmComm;
    Parameters params;
    if(params.parseParameters(argc, argv, AudioProcessorFactory::getAudioProcessorNames()))
    {
        ohmComm = new OHMComm(params);
    }
    else
    {
        ohmComm = new OHMComm(ConfigurationMode::INTERACTIVE);
    }

    ////
    // Startup
    ////
    
    if(!ohmComm->isConfigurationDone(true))
    {
        std::cerr << "Failed to configure OHMComm!" << std::endl;
        return 1;
    }
    ohmComm->startAudioThreads();

    char input;
    // wait for exit
    std::cout << "Type Enter to exit" << std::endl;
    std::cin >> input;

    ohmComm->stopAudioThreads();

    delete ohmComm;
    
    return 0;
}
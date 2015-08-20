/*
 * File:   ConfigurationMode.cpp
 * Author: daniel
 *
 * Created on August 19, 2015, 4:48 PM
 */

#include <algorithm>

#include "ConfigurationMode.h"
#include "AudioHandlerFactory.h"
#include "AudioProcessorFactory.h"
#include "RtAudio.h"

ConfigurationMode::ConfigurationMode()
{
}

ConfigurationMode::~ConfigurationMode()
{
}

const std::pair<std::string, bool> ConfigurationMode::getAudioHandlerConfiguration() const
{
    if(!isConfigurationDone)
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return std::make_pair(audioHandlerName, !useDefaultAudioConfig);
}


const AudioConfiguration ConfigurationMode::getAudioConfiguration() const
{
    if(!isConfigurationDone)
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return audioConfig;
}

const NetworkConfiguration ConfigurationMode::getNetworkConfiguration() const
{
    if(!isConfigurationDone)
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return networkConfig;
}

bool ConfigurationMode::isConfigured() const
{
    return isConfigurationDone;
}

bool ConfigurationMode::getAudioProcessorsConfiguration(std::vector<std::string>& processorNames) const
{
    if(!isConfigurationDone)
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    if(profileProcessors)
    {
        processorNames.reserve(this->processorNames.size());
        std::copy(this->processorNames.begin(), this->processorNames.end(), processorNames.begin());
    }
    return profileProcessors;
}

const std::pair<bool, std::string> ConfigurationMode::getLogToFileConfiguration() const
{
    if(!isConfigurationDone)
    {
        throw std::runtime_error("Configuration was not finished!");
    }
    return std::pair<bool, std::string>(logToFile, logFileName);
}

void ConfigurationMode::createDefaultNetworkConfiguration()
{
    networkConfig.addressOutgoing = "127.0.0.1";
    networkConfig.portIncoming = DEFAULT_NETWORK_PORT;
    networkConfig.portOutgoing = DEFAULT_NETWORK_PORT;
}

ParameterConfiguration::ParameterConfiguration(const Parameters& params)
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
    if(inputDeviceID >= 0 || outputDeviceID >= 0)
    {
        useDefaultAudioConfig = false;
        fillAudioConfiguration(outputDeviceID, inputDeviceID);
    }
    else
    {
        useDefaultAudioConfig = true;
    }
    audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();

    //get network configuration from parameters
    networkConfig.addressOutgoing = params.getParameterValue(Parameters::REMOTE_ADDRESS);
    networkConfig.portIncoming = atoi(params.getParameterValue(Parameters::LOCAL_PORT).c_str());
    networkConfig.portOutgoing = atoi(params.getParameterValue(Parameters::REMOTE_PORT).c_str());

    //get audio-processors from parameters
    profileProcessors = params.isParameterSet(Parameters::PROFILE_PROCESSORS);
    logToFile = params.isParameterSet(Parameters::LOG_TO_FILE);
    logFileName = params.getParameterValue(Parameters::LOG_TO_FILE);

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
    audioConfig.outputDeviceName = audioDevices.getDeviceInfo(outputDeviceID).name;

    audioConfig.inputDeviceID = inputDeviceID;
    audioConfig.inputDeviceName = audioDevices.getDeviceInfo(inputDeviceID).name;
}

bool InteractiveConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }
    //interactive audio configuration
    if (!UserInput::inputBoolean("Load default audio config?"))
    {
        const std::vector<std::string> audioHandlers = AudioHandlerFactory::getAudioHandlerNames();
        //manually configure audio-handler, so manually select it
        if(audioHandlers.size() > 1)
        {
            //only let user choose if there is more than 1 audio-handler
            audioHandlerName = UserInput::selectOption("Select audio handler", audioHandlers, AudioHandlerFactory::getDefaultAudioHandlerName());
        }
        else
        {
            audioHandlerName = *audioHandlers.begin();
        }
        std::cout << "Using AudioHandler: " << audioHandlerName << std::endl;
        interactivelyConfigureAudioDevices();
        useDefaultAudioConfig = false;
    }
    else
    {
        //use RtAudioWrapper as default implementation
        audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    }
    //interactive network configuration
    if (!UserInput::inputBoolean("Load default network config?"))
    {
        interactivelyConfigureNetwork();
    }
    else
    {
        createDefaultNetworkConfiguration();
    }
    interactivelyConfigureProcessors();

    isConfigurationDone = true;
    return true;
}

void InteractiveConfiguration::interactivelyConfigureAudioDevices()
{
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
}

void InteractiveConfiguration::interactivelyConfigureNetwork()
{
    UserInput::printSection("Network configuration");

    //1. remote address
    std::string ipString = UserInput::inputString("1. Input destination IP address");
    networkConfig.addressOutgoing = ipString;

    //2. remote and local ports
    int destPort = UserInput::inputNumber("2. Input destination port", false, false);
    int localPort = UserInput::inputNumber("3. Input local port", false, false);
    networkConfig.portOutgoing = destPort;
    networkConfig.portIncoming = localPort;

    std::cout << "Network configuration set." << std::endl;
}

void InteractiveConfiguration::interactivelyConfigureProcessors()
{
    std::vector<std::string> audioProcessors = AudioProcessorFactory::getAudioProcessorNames();
    //interactive processor configuration
    profileProcessors = UserInput::inputBoolean("Create profiler for audio-processors?");
    logToFile = UserInput::inputBoolean("Log statistics and profiling-results to file?");
    if(logToFile)
    {
        logFileName = UserInput::inputString("Type log-file name");
    }
    // add processors to the process chain
    audioProcessors.push_back("End");
    unsigned int selectedIndex;
    std::cout << "The AudioProcessors should be added in the order they are used on the sending side!" << std::endl;
    while((selectedIndex = UserInput::selectOptionIndex("Select next AudioProcessor to add", audioProcessors, audioProcessors.size()-1)) != audioProcessors.size()-1)
    {
        processorNames.push_back(audioProcessors.at(selectedIndex));
        audioProcessors.at(selectedIndex) = audioProcessors.at(selectedIndex) + " (added)";
    }
}

bool LibraryConfiguration::runConfiguration()
{
    return isConfigured();
}

bool LibraryConfiguration::isConfigured() const
{
    return isAudioConfigured && isNetworkConfigured && isProcessorsConfigured;
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
    std::copy(processorNames.begin(), processorNames.end(), this->processorNames.begin());
    this->profileProcessors = profileProcessors;

    isProcessorsConfigured = true;
}

void LibraryConfiguration::configureLogToFile(const std::string logFileName)
{
    logToFile = true;
    this->logFileName = logFileName;
}

PassiveConfiguration::PassiveConfiguration(const NetworkConfiguration& networkConfig)
{
    this->networkConfig = networkConfig;
}

bool PassiveConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }

    //TODO ask other side for configuration
    return false;
}


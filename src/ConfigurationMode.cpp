/*
 * File:   ConfigurationMode.cpp
 * Author: daniel
 *
 * Created on August 19, 2015, 4:48 PM
 */

#include <algorithm>
#include <fstream>
#include <string>
#include <cctype>
#include <stdlib.h>

#include "ConfigurationMode.h"
#include "AudioHandlerFactory.h"
#include "AudioProcessorFactory.h"
#include "RtAudio.h"
#include "RTCPPackageHandler.h"
#include "UDPWrapper.h"

ConfigurationMode::ConfigurationMode() : audioConfig( {0} ), networkConfig( {0} ), processorNames(std::vector<std::string>(0))
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
    processorNames.reserve(this->processorNames.size());
    for(const std::string& procName : this->processorNames)
    {
        processorNames.push_back(procName);
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
    networkConfig.remoteIPAddress = "127.0.0.1";
    networkConfig.localPort = DEFAULT_NETWORK_PORT;
    networkConfig.remotePort = DEFAULT_NETWORK_PORT;
}

////
//  ParameterConfiguration
////


ParameterConfiguration::ParameterConfiguration(const Parameters& params)
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
    if(inputDeviceID >= 0 || outputDeviceID >= 0)
    {
        useDefaultAudioConfig = false;
        fillAudioConfiguration(outputDeviceID, inputDeviceID);
    }
    else
    {
        useDefaultAudioConfig = true;
    }
    //TODO fix, so force audio-format or sample-rate works without device-parameter set
    if(params.isParameterSet(Parameters::FORCE_AUDIO_FORMAT))
    {
        audioConfig.forceAudioFormatFlag = atoi(params.getParameterValue(Parameters::FORCE_AUDIO_FORMAT).c_str());
    }
    if(params.isParameterSet(Parameters::FORCE_SAMPLE_RATE))
    {
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
    //TODO read from parameters
    return defaultValue;
}

const int ParameterConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    //TODO read from parameters
    return defaultValue;
}

const bool ParameterConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    //TODO read from parameters
    return defaultValue;
}

////
//  InteractiveConfiguration
////

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
        useDefaultAudioConfig = true;
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

const std::string InteractiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    const unsigned char flags = defaultValue.empty() ? 0 : UserInput::INPUT_USE_DEFAULT;
    return UserInput::inputString(message, defaultValue, flags);
}

const int InteractiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    const unsigned char flags = UserInput::INPUT_ALLOW_NEGATIVE | UserInput::INPUT_ALLOW_ZERO | UserInput::INPUT_USE_DEFAULT;
    return UserInput::inputNumber(message, defaultValue, flags);
}

const bool InteractiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    return UserInput::inputBoolean(message, defaultValue, UserInput::INPUT_USE_DEFAULT);
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
    std::cout << "-> Using output Device Name: " << OutputDeviceInfo.name << std::endl;

    //Configure outputDeviceChannels (we always use stereo)
    audioConfig.outputDeviceChannels = 2;

    //sample-rate, audio-format and buffer-size is defined by the enabled Processors so we can skip them

    //Choose input Device
    unsigned int InputDeviceID = UserInput::selectOptionIndex("Choose input Device ID", audioDeviceNames, DefaultInputDeviceID);

    RtAudio::DeviceInfo InputDeviceInfo = AudioDevices.getDeviceInfo(InputDeviceID);

    //Configure ID of the Input Audio Device
    audioConfig.inputDeviceID = InputDeviceID;
    std::cout << "-> Using input Device ID " << audioConfig.inputDeviceID << std::endl;
    std::cout << "-> Using input Device Name: " << InputDeviceInfo.name << std::endl;

    //Configure inputDeviceChannels (we always use stereo)
    audioConfig.inputDeviceChannels = 2;

    //configure whether to force audio-format and sample-rate
    std::vector<std::string> audioFormats = {
        "let audio-handler decide",
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT8, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT16, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT24, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT32, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT32, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT64, true)
    };
    unsigned int selectedAudioFormatIndex = UserInput::selectOptionIndex("Select audio-format", audioFormats, 0);
    if(selectedAudioFormatIndex != 0)
    {
        //must deduct 1 before shifting, because 2^0 = 1 but in the input option 0 is default-value
        audioConfig.forceAudioFormatFlag = 1 << (selectedAudioFormatIndex-1);
    }

    unsigned int selectedSampleRate = UserInput::inputNumber("Input a sample-rate [use 0 for default]", true, false);
    if(selectedSampleRate > 0)
    {
        audioConfig.forceSampleRate = selectedSampleRate;
    }
}

void InteractiveConfiguration::interactivelyConfigureNetwork()
{
    UserInput::printSection("Network configuration");

    //1. remote address
    std::string ipString = UserInput::inputString("1. Input destination IP address");
    networkConfig.remoteIPAddress = ipString;

    //2. remote and local ports
    int destPort = UserInput::inputNumber("2. Input destination port", false, false);
    int localPort = UserInput::inputNumber("3. Input local port", false, false);
    networkConfig.remotePort = destPort;
    networkConfig.localPort = localPort;

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

////
//  LibraryConfiguration
////

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

const int LibraryConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return atoi(customConfig.at(key).data());
}

const bool LibraryConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return atoi(customConfig.at(key).data());
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

////
//  PassiveConfiguration
////

PassiveConfiguration::PassiveConfiguration(const NetworkConfiguration& networkConfig, bool profileProcessors, std::string logFile) :
        ConfigurationMode()
{
    this->useDefaultAudioConfig = false;
    this->audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    this->networkConfig = networkConfig;
    this->profileProcessors = profileProcessors;
    if(!logFile.empty())
    {
        this->logToFile = true;
        this->logFileName = logFile;
    }
    else{
        this->logToFile = false;
    }
}

bool PassiveConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }
    UDPWrapper wrapper(networkConfig);

    RTCPPackageHandler handler;
    RTCPHeader requestHeader(0);  //we do not have a SSID and it doesn't really matter
    ApplicationDefined configRequest("REQC", 0, nullptr, ApplicationDefined::OHMCOMM_CONFIGURATION_REQUEST);

    void* buffer = handler.createApplicationDefinedPackage(requestHeader, configRequest);

    if(wrapper.sendData(buffer, RTCPPackageHandler::getRTCPPackageLength(requestHeader.length)) < (int)RTCPPackageHandler::getRTCPPackageLength(requestHeader.length))
    {
        std::wcerr << wrapper.getLastError() << std::endl;
        return false;
    }

    //we can reuse buffer, because it is large enough (as of RTCPPackageHandler)
    int receivedSize = wrapper.receiveData(buffer, 6000);
    if(receivedSize < 0)
    {
        std::wcerr << wrapper.getLastError() << std::endl;
        return false;
    }
    else if(receivedSize == 0)
    {
        //we didn't receive anything. Don't know if this can actually occur
        return false;
    }
    RTCPHeader responseHeader = handler.readRTCPHeader(buffer, receivedSize);
    if(responseHeader.packageType != RTCP_PACKAGE_APPLICATION_DEFINED)
    {
        std::cerr << "Invalid RTCP response package type: " << responseHeader.packageType << std::endl;
        return false;
    }
    ApplicationDefined configResponse = handler.readApplicationDefinedMessage(buffer, receivedSize, responseHeader);
    if(configResponse.subType != ApplicationDefined::OHMCOMM_CONFIGURATION_RESPONSE)
    {
        std::cerr << "Invalid RTCP sub-type for response package: " << configResponse.name << " " << configResponse.subType << std::endl;
        return false;
    }
    //we need to receive audio-configuration and processor-names
    ConfigurationMessage receivedMessage = PassiveConfiguration::readConfigurationMessage(configResponse.data, configResponse.dataLength);

    std::cout << "Passive Configuration received ... " << std::endl;

    //TODO force buffer frames? Should be distinct from combination of sample-rate and audio-processors
    std::cout << "Received audio-format: " << AudioConfiguration::getAudioFormatDescription(receivedMessage.audioFormat, false) << std::endl;;
    audioConfig.forceAudioFormatFlag = receivedMessage.audioFormat;
    std::cout << "Received sample-rate: " << receivedMessage.sampleRate << std::endl;
    audioConfig.forceSampleRate = receivedMessage.sampleRate;
    std::cout << "Received channels: " << receivedMessage.nChannels << std::endl;
    audioConfig.inputDeviceChannels = receivedMessage.nChannels;
    audioConfig.outputDeviceChannels = receivedMessage.nChannels;
    std::cout << "\tReceived audio-processors: ";
    for(std::string& procName : receivedMessage.processorNames)
    {
        std::cout << procName << ' ';
    }
    std::cout << std::endl;
    processorNames.reserve(receivedMessage.processorNames.size());
    std::copy(receivedMessage.processorNames.begin(), receivedMessage.processorNames.end(), processorNames.begin());

    this->isConfigurationDone = true;
    return true;
}

const std::string PassiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    return defaultValue;
}

const int PassiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    return defaultValue;
}

const bool PassiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    return defaultValue;
}

const PassiveConfiguration::ConfigurationMessage PassiveConfiguration::readConfigurationMessage(void* buffer, unsigned int bufferSize)
{
    ConfigurationMessage message = *((ConfigurationMessage*)buffer);
    //we filled the vector with trash, so we initialize a new one
    message.processorNames = {};
    message.processorNames.reserve(message.numProcessorNames);
    //get the pointer to the first processor-name
    char* procNamePtr = (char*)buffer + CONFIGURATION_MESSAGE_SIZE;
    for(uint8_t numProc = 0; numProc < message.numProcessorNames; numProc++)
    {
        std::string nextProcName = std::string(procNamePtr);
        message.processorNames.push_back(nextProcName);
        //skip processor-name and zero-termination
        procNamePtr += nextProcName.size() + 1;
    }
    return message;
}

unsigned int PassiveConfiguration::writeConfigurationMessage(void* buffer, unsigned int maxBufferSize, ConfigurationMessage& configMessage)
{
    if(maxBufferSize < CONFIGURATION_MESSAGE_SIZE)
    {
        return -1;
    }
    unsigned int writtenBytes = 0;
    configMessage.numProcessorNames = configMessage.processorNames.size();
    //copy the configuration-message to the output-buffer
    *((ConfigurationMessage*)buffer) = configMessage;
    writtenBytes = CONFIGURATION_MESSAGE_SIZE;
    //we can't send a vector, so we map it to a string-array
    for(uint8_t procIndex = 0; procIndex < configMessage.numProcessorNames; procIndex++)
    {
        std::string nextProcName = configMessage.processorNames[procIndex];
        //copy the string into the package
        writtenBytes += nextProcName.copy((char*)buffer + writtenBytes, nextProcName.size(), 0);
        //terminate name with a zero-byte
        *((char*)buffer + writtenBytes) = '\0';
        writtenBytes += 1;
    }
    return writtenBytes;
}

////
//  FileConfiguration
////

FileConfiguration::FileConfiguration(const std::string fileName) : ConfigurationMode(), configFile(fileName)
{

}

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   return std::string(wsfront,std::find_if_not(s.rbegin(),std::string::const_reverse_iterator(wsfront),[](int c){return std::isspace(c);}).base());
}

inline std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

bool FileConfiguration::runConfiguration()
{
    if(isConfigurationDone)
        return true;

    //read configuration-file
    std::fstream stream(configFile.data(), std::fstream::in);
    stream.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    std::string line;
    unsigned int index;
    std::string key, value;
    while(true)
    {
        line.clear();
        std::getline(stream, line);
        if(stream.eof()) break;
        if(stream.gcount() == 0) continue;
        if(line[0] == '#') continue;

        //read key
        index = line.find('=');
        if(index == std::string::npos)
        {
            std::cerr << "Invalid configuration line: " << line << std::endl;
            continue;
        }
        key = trim(line.substr(0, index));
        index++;
        //read value
        value = trim(line.substr(index));
        if(value[0] == '"')
        {
            value = value.substr(1, value.size()-2);
            //unescape escapes
            replaceAll(value, "\\\"", "\"");
        }
        else if(value.compare("true") == 0)
            value = "1";
        else if(value.compare("false") == 0)
            value = "0";
        //save value
        if(key.compare(Parameters::AUDIO_PROCESSOR.longName) == 0)
            processorNames.push_back(trim(value));
        else
            customConfig[key] = trim(value);
    }
    stream.close();

    //interpret config
    if(customConfig.find(Parameters::AUDIO_HANDLER.longName) != customConfig.end())
    {
        audioHandlerName = trim(customConfig.at(Parameters::AUDIO_HANDLER.longName));
    }
    else
    {
        audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
    }
    //audio-configuration
    if(customConfig.find(Parameters::INPUT_DEVICE.longName) != customConfig.end())
    {
        useDefaultAudioConfig = false;
        audioConfig.inputDeviceID = atoi(customConfig.at(Parameters::INPUT_DEVICE.longName).data());
    }
    if(customConfig.find(Parameters::OUTPUT_DEVICE.longName) != customConfig.end())
    {
        useDefaultAudioConfig = false;
        audioConfig.outputDeviceID = atoi(customConfig.at(Parameters::OUTPUT_DEVICE.longName).data());
    }
    if(customConfig.find(Parameters::FORCE_AUDIO_FORMAT.longName) != customConfig.end())
    {
        useDefaultAudioConfig = false;
        audioConfig.forceAudioFormatFlag = atoi(customConfig.at(Parameters::FORCE_AUDIO_FORMAT.longName).data());
    }
    if(customConfig.find(Parameters::FORCE_SAMPLE_RATE.longName) != customConfig.end())
    {
        useDefaultAudioConfig = false;
        audioConfig.forceSampleRate = atoi(customConfig.at(Parameters::FORCE_SAMPLE_RATE.longName).data());
    }
    audioConfig.inputDeviceChannels = 2;
    audioConfig.outputDeviceChannels = 2;

    //network-configuration
    networkConfig.remoteIPAddress = customConfig.at(Parameters::REMOTE_ADDRESS.longName);
    if(customConfig.find(Parameters::REMOTE_PORT.longName) != customConfig.end())
        networkConfig.remotePort = atoi(customConfig.at(Parameters::REMOTE_PORT.longName).data());
    else
        networkConfig.remotePort = DEFAULT_NETWORK_PORT;
    if(customConfig.find(Parameters::LOCAL_PORT.longName) != customConfig.end())
        networkConfig.localPort = atoi(customConfig.at(Parameters::LOCAL_PORT.longName).data());
    else
        networkConfig.localPort = DEFAULT_NETWORK_PORT;
    //audio-processors are read above
    profileProcessors = customConfig.find(Parameters::PROFILE_PROCESSORS.longName) != customConfig.end();
    if(customConfig.find(Parameters::LOG_TO_FILE.longName) != customConfig.end())
    {
        logToFile = true;
        logFileName = customConfig.at(Parameters::LOG_TO_FILE.longName);
    }

    isConfigurationDone = true;
    return true;
}

const std::string FileConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return customConfig.at(key);
}

const int FileConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return atoi(customConfig.at(key).data());
}

const bool FileConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    if(customConfig.find(key) == customConfig.end())
    {
        return defaultValue;
    }
    return atoi(customConfig.at(key).data());
}

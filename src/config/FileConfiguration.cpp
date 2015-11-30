/* 
 * File:   FileConfiguration.cpp
 * Author: daniel
 * 
 * Created on November 30, 2015, 5:36 PM
 */

#include "config/FileConfiguration.h"

#include "Parameters.h"
#include "AudioHandlerFactory.h"
#include "AudioProcessorFactory.h"

FileConfiguration::FileConfiguration(const std::string fileName) : ConfigurationMode(), configFile(fileName)
{
    createDefaultNetworkConfiguration();
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

    try
    {
        std::cout << "Reading configuration file... " << configFile << std::endl;
        //read configuration-file
        std::fstream stream(configFile, std::fstream::in);
        stream.exceptions ( std::ios::badbit );
        std::string line;
        unsigned int index;
        std::string key, value;
        if(!stream.is_open())
        {
            throw std::ios::failure("Could not open file!");
        }
        while(true)
        {
            std::getline(stream, line);
            if(stream.eof()) break;
            if(line.empty()) continue;
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
            if(key.compare(Parameters::AUDIO_PROCESSOR->longName) == 0)
                processorNames.push_back(trim(value));
            else
                customConfig[key] = trim(value);
        }
        stream.close();
        
        //interpret config
        if(customConfig.find(Parameters::AUDIO_HANDLER->longName) != customConfig.end())
        {
            audioHandlerName = trim(customConfig.at(Parameters::AUDIO_HANDLER->longName));
        }
        else
        {
            audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
        }
        //audio-configuration
        if(customConfig.find(Parameters::INPUT_DEVICE->longName) != customConfig.end())
        {
            useDefaultAudioConfig = false;
            audioConfig.inputDeviceID = atoi(customConfig.at(Parameters::INPUT_DEVICE->longName).data());
        }
        if(customConfig.find(Parameters::OUTPUT_DEVICE->longName) != customConfig.end())
        {
            useDefaultAudioConfig = false;
            audioConfig.outputDeviceID = atoi(customConfig.at(Parameters::OUTPUT_DEVICE->longName).data());
        }
        if(customConfig.find(Parameters::FORCE_AUDIO_FORMAT->longName) != customConfig.end())
        {
            useDefaultAudioConfig = false;
            audioConfig.forceAudioFormatFlag = atoi(customConfig.at(Parameters::FORCE_AUDIO_FORMAT->longName).data());
            audioConfig.audioFormatFlag = atoi(customConfig.at(Parameters::FORCE_AUDIO_FORMAT->longName).data());
        }
        if(customConfig.find(Parameters::FORCE_SAMPLE_RATE->longName) != customConfig.end())
        {
            useDefaultAudioConfig = false;
            audioConfig.forceSampleRate = atoi(customConfig.at(Parameters::FORCE_SAMPLE_RATE->longName).data());
            audioConfig.sampleRate = atoi(customConfig.at(Parameters::FORCE_SAMPLE_RATE->longName).data());
        }
        audioConfig.inputDeviceChannels = 2;
        audioConfig.outputDeviceChannels = 2;

        //network-configuration
        if(customConfig.find(Parameters::REMOTE_ADDRESS->longName) != customConfig.end())
            networkConfig.remoteIPAddress = customConfig.at(Parameters::REMOTE_ADDRESS->longName);
        if(customConfig.find(Parameters::REMOTE_PORT->longName) != customConfig.end())
            networkConfig.remotePort = atoi(customConfig.at(Parameters::REMOTE_PORT->longName).data());
        if(customConfig.find(Parameters::LOCAL_PORT->longName) != customConfig.end())
            networkConfig.localPort = atoi(customConfig.at(Parameters::LOCAL_PORT->longName).data());
        //audio-processors are read above
        profileProcessors = customConfig.find(Parameters::PROFILE_PROCESSORS->longName) != customConfig.end();
        if(customConfig.find(Parameters::LOG_TO_FILE->longName) != customConfig.end())
        {
            logToFile = true;
            logFileName = customConfig.at(Parameters::LOG_TO_FILE->longName);
        }
        waitForConfigurationRequest = customConfig.find(Parameters::WAIT_FOR_PASSIVE_CONFIG->longName) != customConfig.end();
    }
    catch(std::ios_base::failure f)
    {
        std::cerr << "Failed to read configuration-file!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        std::cerr << f.what() << std::endl;
        return false;
    }
    std::cout << "Configuration-file read" << std::endl;
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

const bool FileConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return customConfig.find(key) != customConfig.end();
}

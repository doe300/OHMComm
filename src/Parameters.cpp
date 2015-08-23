/*
 * File:   Parameter.cpp
 * Author: daniel
 *
 * Created on July 22, 2015, 1:57 PM
 */

#include "Parameters.h"
#include <iomanip>
#include <string.h>

const Parameter Parameters::HELP(ParameterCategory::GENERAL, 'h', "help", "Print this help page and exit");
const Parameter Parameters::PASSIVE_CONFIGURATION(ParameterCategory::GENERAL, 'P', "passive", "Enables passive configuration. The communication partner will decide the audio-configuration");
const Parameter Parameters::LOG_TO_FILE(ParameterCategory::GENERAL, 'f', "log-file", "Log statistics and profiling-information to file.", "OHMComm.log");
const Parameter Parameters::INPUT_DEVICE(ParameterCategory::AUDIO, 'i', "input-device-id", "The id of the device used for audio-input. This value will fall back to the library-default", "");
const Parameter Parameters::OUTPUT_DEVICE(ParameterCategory::AUDIO, 'o', "output-device-id", "The id of the device used for audio-output. This value will fall back to the library-default", "");
const Parameter Parameters::FORCE_AUDIO_FORMAT(ParameterCategory::AUDIO, 'A', "audio-format", "Forces the given audio-format to be used. For a list of audio-formats see below. Currently only works in conjunction with -i or -o.", "");
const Parameter Parameters::FORCE_SAMPLE_RATE(ParameterCategory::AUDIO, 'S', "sample-rate", "Forces the given sample-rate to be used, i.e. 44100. Currently only works in conjunction with -i or -o.", "");
const Parameter Parameters::REMOTE_ADDRESS(ParameterCategory::NETWORK, true, 'r', "remote-address", "The IP address of the computer to connect to", "", true);
const Parameter Parameters::REMOTE_PORT(ParameterCategory::NETWORK, true, 'p', "remote-port", "The port of the remote computer", std::to_string(DEFAULT_NETWORK_PORT), true);
const Parameter Parameters::LOCAL_PORT(ParameterCategory::NETWORK, true, 'l', "local-port", "The local port to listen on", std::to_string(DEFAULT_NETWORK_PORT), true);
const Parameter Parameters::AUDIO_PROCESSOR(ParameterCategory::PROCESSORS, 'a', "add-processor", "The name of the audio-processor to add", "");
const Parameter Parameters::PROFILE_PROCESSORS(ParameterCategory::PROCESSORS, 't', "profile-processors", "Enables profiling of the the execution time of audio-processors");

//TODO allow for adding processor-specific parameters
//they would need to be added before reading the command-line arguments.
//Which means, before the processors are initialized.
//Could they be added via global code?
const std::vector<const Parameter*> Parameters::availableParameters = {
    //General
    &HELP, &LOG_TO_FILE, &PASSIVE_CONFIGURATION,
    //Audio-config
    &INPUT_DEVICE, &OUTPUT_DEVICE, &FORCE_AUDIO_FORMAT, &FORCE_SAMPLE_RATE,
    //Network-config
    &REMOTE_ADDRESS, &REMOTE_PORT, &LOCAL_PORT,
    //Processor-config
    &AUDIO_PROCESSOR, &PROFILE_PROCESSORS
};

Parameters::Parameters()
{
}

bool Parameters::parseParameters(int argc, char* argv[], const std::vector<std::string> allProcessorNames)
{
    //argv[0] is the name of the program
    if(argc <= 1)
    {
        //no parameters to handle
        return false;
    }
    readParameters.reserve(argc-1);
    processorNames.reserve(5);
    for(int index = 1; index < argc; index++)
    {
        char* paramText = argv[index];
        const Parameter* param = nullptr;
        //1. check for '-' or '--'
        unsigned long numSlashes = strspn(paramText, "--");
        if(numSlashes == 0 || numSlashes > 2)
        {
            //Invalid argument syntax - print message and continue
            std::cout << "Invalid syntax for parameter: " << paramText << std::endl;
            std::cout << "See \"OHMComm --help\" for accepted parameters" << std::endl;
        }
        //2. get correct parameter
        if(numSlashes == 1)
        {
            char shortParam = paramText[1];
            for(const Parameter* p : availableParameters)
            {
                if(p->shortName == shortParam)
                {
                    param = p;
                    break;
                }
            }
        }
        else
        {
            unsigned int posEqualsSign = std::string(paramText).find("=");
            std::string longParam = std::string(paramText).substr(2, posEqualsSign-2);
            for(const Parameter* p : availableParameters)
            {
                if(p->longName.compare(longParam) == 0)
                {
                    param = p;
                    break;
                }
            }
        }
        if(param == nullptr)
        {
            //Unrecognized argument - print message and continue
            std::cout << "Parameter \"" << paramText << "\" not found, skipping." << std::endl;
            std::cout << "See \"OHMComm --help\" for a list of all parameters" << std::endl;
        }
        std::string value;
        //3. read value, if any
        if(param->hasValue)
        {
            char* paramValue = strstr(paramText, "=");
            if(paramValue != nullptr)
            {
                //we need to skip the '='
                paramValue = paramValue + 1;
                //value set via parameter
                value = std::string(paramValue);
            }
            if(value.empty())
            {
                //No value set
                std::cout << "No value set for parameter \"" << param->longName << "\", skipping." << std::endl;
                std::cout << "See \"OHMComm --help\" for a list of parameters and their accepted values" << std::endl;
            }
        }
        //4. set mapping
        readParameters.push_back(ParameterValue(param, value));
        //4.1 if audio-processor, add to list
        if(param == &AUDIO_PROCESSOR)
        {
            processorNames.push_back(value);
        }
    }
    //print help page and exit if requested
    if(isParameterSet(HELP))
    {
        printHelpPage(allProcessorNames);
        exit(0);
    }
    //check if all required parameters are set or at least have a default-value
    for(const Parameter* avParam :availableParameters)
    {
        if(avParam->required == true && !isParameterSet(*avParam) && avParam->defaultValue.empty())
        {
            std::cout << "No value set for required parameter: " << avParam->longName << std::endl;
            std::cout << "See \"OHMComm --help\" for a list of parameters and their possible values" << std::endl;
            //we most likely can't run with some essential parameter missing
            std::cout << "Aborting." << std::endl;
            exit(1);
        }
    }
    return true;
}

const std::vector<std::string> Parameters::getAudioProcessors() const
{
    return processorNames;
}

void Parameters::printHelpPage(const std::vector<std::string> allProcessorNames) const
{
    std::cout << "OHMComm peer-to-peer voice-over-IP communication program running in version " << OHMCOMM_VERSION << std::endl;
    std::cout << "Usage: OHMComm [option]" << std::endl;
    std::cout << "When run without command-line arguments, the program will start in interactive mode." << std::endl;
    std::cout << std::endl;

    std::cout << "General configuration:" << std::endl;
    for(const Parameter* param : availableParameters)
    {
        if(param->category == ParameterCategory::GENERAL)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Audio configuration:" << std::endl;
    for(const Parameter* param : availableParameters)
    {
        if(param->category == ParameterCategory::AUDIO)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Network configuration:" << std::endl;
    for(const Parameter* param : availableParameters)
    {
        if(param->category == ParameterCategory::NETWORK)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Processor configuration:" << std::endl;
    for(const Parameter* param : availableParameters)
    {
        if(param->category == ParameterCategory::PROCESSORS)
        {
            printParameterHelp(param);
        }
    }
    std::cout << std::endl;
    
    //print AudioProcessors
    std::cout << "Currently available audio-processors are: " << std::endl;
    for(const std::string& name : allProcessorNames)
    {
       std::cout << std::setw(tabSize) << ' ' << name << std::endl;
    }
    std::cout << std::endl;
    
    //print audio-formats
    std::cout << "Available audio-formats (use the number as parameter): " << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT8 
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT8) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT16 
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT16) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT24 
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT24) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT32 
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT32) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_FLOAT32 
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT32) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_FLOAT64 
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT64) << std::endl;
}

bool Parameters::isParameterSet(const Parameter& param) const
{
    for(const ParameterValue& val: readParameters)
    {
        if(val.parameter == &param)
        {
            return true;
        }
    }
    return false;
}

std::string Parameters::getParameterValue(const Parameter& param) const
{
    for(const ParameterValue& val: readParameters)
    {
        if(val.parameter->longName == param.longName)
        {
            return val.value;
        }
    }
    return param.defaultValue;
}



std::string Parameters::getCategoryName(const ParameterCategory& category) const
{
    switch(category)
    {
        case ParameterCategory::GENERAL:
            return "General";
        case ParameterCategory::AUDIO:
            return "Audio";
        case ParameterCategory::NETWORK:
            return "Network";
        case ParameterCategory::PROCESSORS:
        return "Processors";
    }
    return "Other";
}

void Parameters::printParameterHelp(const Parameter* param) const
{
    //Offset to build column-like formatting
    unsigned int offset = 0;
    if(param->shortName != 0)
    {
        //start column at offset tabSize
        std::cout << std::setw(tabSize) << ' ' <<  '-' << param->shortName;
        offset = tabSize + 1 + 1;
    }
    if(!param->longName.empty())
    {
        //start column at offset 12
        std::cout << std::setw(tabSize) << ' ' << "--" << param->longName;
        offset += tabSize + param->longName.length();
    }
    if(param->hasValue)
    {
        std::cout << "=value";
        offset+= std::string("=value").length();
        if(!param->defaultValue.empty())
        {
            std::cout << " [" << param->defaultValue << "]";
            offset += 3 + param->defaultValue.length();
        }
    }
    if(!param->infoText.empty())
    {
        //start column at offset 45
        std::cout << std::setw(45 - offset) << ' ';
        if(param->required)
        {
            std::cout << "Required. ";
        }
        std::cout << param->infoText;
    }
    std::cout << std::endl;
}

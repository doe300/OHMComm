/*
 * File:   Parameter.cpp
 * Author: daniel
 *
 * Created on July 22, 2015, 1:57 PM
 */
#include <iomanip>
#include <string.h>
#include <fstream>

#include "Parameters.h"
#include "Utility.h"
#include "Logger.h"

using namespace ohmcomm;

std::list<Parameter> Parameters::availableParameters = {};
//parameters must be registered after initializing the availableParameters
const Parameter* Parameters::HELP = Parameters::registerParameter(Parameter(ParameterCategory::GENERAL, 'h', "help", "Print this help page and exit"));
const Parameter* Parameters::LIST_LOCAL_ADDRESSES = Parameters::registerParameter(Parameter(ParameterCategory::NETWORK, 'n', "list-addresses", "Display the local addresses and exit"));
const Parameter* Parameters::LIST_AUDIO_DEVICES = Parameters::registerParameter(Parameter(ParameterCategory::AUDIO, 'D', "list-devices", "List all available audio-deviced for the selected audio-library and exist."));
const Parameter* Parameters::SIP_LOCAL_PORT = Parameters::registerParameter(Parameter(ParameterCategory::GENERAL, Parameter::FLAG_CONFIGURATION_MODE|Parameter::FLAG_HAS_VALUE, 's', "sip-local-port", "Enables signaling and configuration via SIP. The value is the local SIP-port", "5060"));
const Parameter* Parameters::SIP_REMOTE_PORT = Parameters::registerParameter(Parameter(ParameterCategory::GENERAL, Parameter::FLAG_CONFIGURATION_MODE|Parameter::FLAG_HAS_VALUE, 'c', "sip-remote-port", "Enables signaling and configuration via SIP. The value is the remote SIP-port", "5060"));
const Parameter* Parameters::SIP_REGISTER_USER = Parameters::registerParameter(Parameter(ParameterCategory::GENERAL, Parameter::FLAG_CONFIGURATION_MODE|Parameter::FLAG_HAS_VALUE, 'R', "sip-register-user", "Enables signaling and configuration via SIP. The value determines the user to register as with the given server", ""));
const Parameter* Parameters::LOG_TO_FILE = Parameters::registerParameter(Parameter(ParameterCategory::GENERAL, 'f', "log-file", "Log statistics and profiling-information to file.", "OHMComm.log"));
const Parameter* Parameters::AUDIO_HANDLER = Parameters::registerParameter(Parameter(ParameterCategory::AUDIO, 'H', "audio-handler", "Use this specific audio-handler. Defaults to the program-default audio-handler", ""));
const Parameter* Parameters::INPUT_DEVICE = Parameters::registerParameter(Parameter(ParameterCategory::AUDIO, 'i', "input-device-id", "The id of the device used for audio-input. This value will fall back to the library-default", ""));
const Parameter* Parameters::OUTPUT_DEVICE = Parameters::registerParameter(Parameter(ParameterCategory::AUDIO, 'o', "output-device-id", "The id of the device used for audio-output. This value will fall back to the library-default", ""));
const Parameter* Parameters::FORCE_AUDIO_FORMAT = Parameters::registerParameter(Parameter(ParameterCategory::AUDIO, 'A', "audio-format", "Forces the given audio-format to be used. For a list of audio-formats see below. Currently only works in conjunction with -i or -o.", ""));
const Parameter* Parameters::FORCE_SAMPLE_RATE = Parameters::registerParameter(Parameter(ParameterCategory::AUDIO, 'S', "sample-rate", "Forces the given sample-rate to be used, i.e. 44100. Currently only works in conjunction with -i or -o.", ""));
const Parameter* Parameters::REMOTE_ADDRESS = Parameters::registerParameter(Parameter(ParameterCategory::NETWORK, Parameter::FLAG_REQUIRED|Parameter::FLAG_HAS_VALUE, 'r', "remote-address", "The IP address of the computer to connect to", ""));
const Parameter* Parameters::REMOTE_PORT = Parameters::registerParameter(Parameter(ParameterCategory::NETWORK, Parameter::FLAG_REQUIRED|Parameter::FLAG_HAS_VALUE, 'p', "remote-port", "The port of the remote computer", std::to_string(DEFAULT_NETWORK_PORT)));
const Parameter* Parameters::LOCAL_PORT = Parameters::registerParameter(Parameter(ParameterCategory::NETWORK, Parameter::FLAG_REQUIRED|Parameter::FLAG_HAS_VALUE, 'l', "local-port", "The local port to listen on", std::to_string(DEFAULT_NETWORK_PORT)));
const Parameter* Parameters::AUDIO_PROCESSOR = Parameters::registerParameter(Parameter(ParameterCategory::PROCESSORS, 'a', "add-processor", "The name of the audio-processor to add", ""));
const Parameter* Parameters::PROFILE_PROCESSORS = Parameters::registerParameter(Parameter(ParameterCategory::PROCESSORS, 't', "profile-processors", "Enables profiling of the the execution time of audio-processors"));
const Parameter* Parameters::ENABLE_DTX = Parameters::registerParameter(Parameter(ParameterCategory::NETWORK, 'd', "enable-dtx", "Enables DTX to not send any packages, if silence is detected."));
const Parameter* Parameters::ENABLE_FEC = Parameters::registerParameter(Parameter(ParameterCategory::NETWORK, 'e', "enable-fec", "Enables FEC to include forward-error-correction data into supported formats."));

const Parameter* Parameters::SDES_CNAME = Parameters::registerParameter(Parameter(ParameterCategory::SOURCE_DESCRIPTION, 'C', "sdes-cname", "The SDES CNAME (device name)", ""));
const Parameter* Parameters::SDES_EMAIL = Parameters::registerParameter(Parameter(ParameterCategory::SOURCE_DESCRIPTION, 'E', "sdes-email", "The SDES EMAIL (email-address)", ""));
const Parameter* Parameters::SDES_LOC = Parameters::registerParameter(Parameter(ParameterCategory::SOURCE_DESCRIPTION, 'L', "sdes-location", "The SDES LOC (geographic location)", ""));
const Parameter* Parameters::SDES_NAME = Parameters::registerParameter(Parameter(ParameterCategory::SOURCE_DESCRIPTION, 'N', "sdes-name", "The SDES NAME (participant name)", ""));
const Parameter* Parameters::SDES_NOTE = Parameters::registerParameter(Parameter(ParameterCategory::SOURCE_DESCRIPTION, 'M', "sdes-note", "The SDES NOTE (some arbitrary note)", ""));
const Parameter* Parameters::SDES_PHONE = Parameters::registerParameter(Parameter(ParameterCategory::SOURCE_DESCRIPTION, 'T', "sdes-phone", "The SDES PHONE (participant phone number)", ""));

const Parameter* Parameters::registerParameter(Parameter&& param)
{
    //make sure, neither short name nor long name are reused
    std::list<Parameter>::iterator insertPos = availableParameters.begin();
    while(insertPos != availableParameters.end())
    {
        if((*insertPos).shortName == param.shortName)
        {
            ohmcomm::error("Parameters") << "Short parameter name '" << param.shortName << "' already in use for parameter '" << (*insertPos).longName << "'!" << ohmcomm::endl;
            return nullptr;
        }
        if((*insertPos).longName == param.longName)
        {
            ohmcomm::error("Parameters") << "Long parameter name '" << param.longName << "' already in use!" << ohmcomm::endl;
            return nullptr;
        }
        //select position to insert Parameter into - choose first entry in list behind the Parameter
        if(insertPos == availableParameters.end() || (*insertPos) > param)
        {
            //we have the first entry behind the current Parameter - go one entry back and break loop
            break;
        }
        //go to next entry
        insertPos++;
    }
    //insert into list at calculated position to preserve sort-order
    availableParameters.emplace(insertPos, std::move(param));
    //return pointer to the added element
    return getParameter(param.longName);
}

const Parameter* Parameters::getParameter(const std::string paramName)
{
    for(const Parameter& p : availableParameters)
    {
        if(p.longName == paramName)
            return &p;
    }
    return nullptr;
}


Parameters::Parameters(const std::vector<std::string> availableHandlerNames, const std::vector<std::string> availableProcessorNames) :
    allAudioHandlerNames(availableHandlerNames), allProcessorNames(availableProcessorNames)
{
}

bool Parameters::parseParameters(int argc, char* argv[])
{
    //argv[0] is the name of the program
    if(argc <= 1)
    {
        //no parameters to handle
        return false;
    }
    //if run with a single parameter not starting with '-', use it as configuration-file
    if(argc == 2 && argv[1][0] != '-')
    {
        if(!parseParametersFromFile(std::string(argv[1])))
        {
            return false;
        }
    }
    else
    {
        parseParametersCommandLine(argc, argv);
    }
    //print help page and exit if requested
    if(isParameterSet(HELP))
    {
        printHelpPage();
        exit(0);
    }
    if(isParameterSet(LIST_LOCAL_ADDRESSES))
    {
        const std::string externalAddress = Utility::getLocalIPAddress(Utility::AddressType::ADDRESS_INTERNET);
        std::cout << std::endl;
        std::cout << "Listing local addresses:" << std::endl;
        std::cout << std::setw(tabSize) << ' ' << "Loopback: " << Utility::getLocalIPAddress(Utility::AddressType::ADDRESS_LOOPBACK) << std::endl;
        std::cout << std::setw(tabSize) << ' ' << "Local: " << Utility::getLocalIPAddress(Utility::AddressType::ADDRESS_LOCAL_NETWORK) << std::endl;
        std::cout << std::setw(tabSize) << ' ' << "External: " << externalAddress << std::endl;
        exit(0);
    }
    if(isParameterSet(LIST_AUDIO_DEVICES))
    {
        //skip the required-parameter check
        return true;
    }
    //if we set another configuration-mode, we can skip the required-parameter check
    for(const ParameterValue& paramValue : readParameters)
    {
        if(paramValue.parameter->isConfigurationMode())
        {
            return true;
        }
    }
    //check if all required parameters are set or at least have a default-value
    for(const Parameter& avParam :availableParameters)
    {
        if(avParam.isRequired() && !isParameterSet(&avParam) && !avParam.hasDefaultValue())
        {
            ohmcomm::error("Parameters") << "No value set for required parameter: " << avParam.longName << ohmcomm::endl;
            ohmcomm::error("Parameters") << "See \"OHMComm --help\" for a list of parameters and their possible values" << ohmcomm::endl;
            //we most likely can't run with some essential parameter missing
            ohmcomm::error("Parameters") << "Aborting." << ohmcomm::endl;
            exit(1);
        }
    }
    return true;
}

void Parameters::setParameter(const Parameter* param, const std::string& value)
{
    readParameters.push_back(ParameterValue(param, value));
    //4.1 if audio-processor, add to list
    if(param == AUDIO_PROCESSOR)
    {
        processorNames.push_back(value);
    }
}

const std::vector<std::string> Parameters::getAudioProcessors() const
{
    return processorNames;
}

void Parameters::printHelpPage() const
{
    std::cout << "OHMComm peer-to-peer voice-over-IP communication program running in version " << OHMCOMM_VERSION << std::endl;
    std::cout << "Usage: OHMComm [option]" << std::endl
            << std::setw(tabSize) << ' ' << "or OHMComm config-file" << std::endl
            << std::setw(tabSize) << ' ' << "or OHMComm" << std::endl;
    std::cout << "When run without command-line arguments, the program will start in interactive mode." << std::endl;
    std::cout << "Running the program with a file-path as single argument, the settings will be read from the specified file."
            << " The path will be relative to the current working directory." << std::endl;
    std::cout << std::endl;

    std::cout << "General configuration:" << std::endl;
    for(const Parameter& param : availableParameters)
    {
        if(param.category == ParameterCategory::GENERAL)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Audio configuration:" << std::endl;
    for(const Parameter& param : availableParameters)
    {
        if(param.category == ParameterCategory::AUDIO)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Network configuration:" << std::endl;
    for(const Parameter& param : availableParameters)
    {
        if(param.category == ParameterCategory::NETWORK)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Processor configuration:" << std::endl;
    for(const Parameter& param : availableParameters)
    {
        if(param.category == ParameterCategory::PROCESSORS)
        {
            printParameterHelp(param);
        }
    }
    std::cout << "Source Description values:" << std::endl;
    for(const Parameter& param : availableParameters)
    {
        if(param.category == ParameterCategory::SOURCE_DESCRIPTION)
        {
            printParameterHelp(param);
        }
    }
    std::cout << std::endl;

    //print AudioHandlers
    std::cout << "Currently available audio-handlers are: " << std::endl;
    for(const std::string& name : allAudioHandlerNames)
    {
       std::cout << std::setw(tabSize) << ' ' << name << std::endl;
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
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT8, true) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT16
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT16, true) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT24
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT24, true) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_SINT32
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT32, true) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_FLOAT32
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT32, true) << std::endl;
    std::cout << std::setw(tabSize) << ' ' << AudioConfiguration::AUDIO_FORMAT_FLOAT64
            << ": " << AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT64, true) << std::endl;
}

bool Parameters::isParameterSet(const Parameter* param) const
{
    for(const ParameterValue& val: readParameters)
    {
        if(val.parameter == param)
        {
            return true;
        }
    }
    return false;
}

const std::string Parameters::getParameterValue(const Parameter* param) const
{
    for(const ParameterValue& val: readParameters)
    {
        if(val.parameter == param)
        {
            return val.value;
        }
    }
    return param->defaultValue;
}

void Parameters::printParameterHelp(const Parameter& param) const
{
    //Offset to build column-like formatting
    unsigned int offset = 0;
    if(param.shortName != 0)
    {
        //start column at offset tabSize
        std::cout << std::setw(tabSize) << ' ' <<  '-' << param.shortName;
        offset = tabSize + 1 + 1;
    }
    if(!param.longName.empty())
    {
        //start column at offset 12
        std::cout << std::setw(tabSize) << ' ' << "--" << param.longName;
        offset += tabSize + param.longName.length();
    }
    if(param.hasValue())
    {
        std::cout << "=value";
        offset+= std::string("=value").length();
        if(param.hasDefaultValue())
        {
            std::cout << " [" << param.defaultValue << "]";
            offset += 3 + param.defaultValue.length();
        }
    }
    if(!param.infoText.empty())
    {
        //start column at offset 45
        std::cout << std::setw(45 - offset) << ' ';
        if(param.isRequired())
        {
            std::cout << "Required. ";
        }
        std::cout << param.infoText;
    }
    std::cout << std::endl;
}

void Parameters::parseParametersCommandLine(int argc, char* argv[])
{
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
            ohmcomm::warn("Parameters") << "Invalid syntax for parameter: " << paramText << ohmcomm::endl;
            ohmcomm::warn("Parameters") << "See \"OHMComm --help\" for accepted parameters" << ohmcomm::endl;
            continue;
        }
        //2. get correct parameter
        if(numSlashes == 1)
        {
            char shortParam = paramText[1];
            for(const Parameter& p : availableParameters)
            {
                if(p.shortName == shortParam)
                {
                    param = &p;
                    break;
                }
            }
        }
        else
        {
            unsigned int posEqualsSign = std::string(paramText).find("=");
            std::string longParam = std::string(paramText).substr(2, posEqualsSign-2);
            for(const Parameter& p : availableParameters)
            {
                if(p.longName.compare(longParam) == 0)
                {
                    param = &p;
                    break;
                }
            }
        }
        if(param == nullptr)
        {
            //Unrecognized argument - print message and continue
            ohmcomm::warn("Parameters") << "Parameter \"" << paramText << "\" not found, skipping." << ohmcomm::endl;
            ohmcomm::warn("Parameters") << "See \"OHMComm --help\" for a list of all parameters" << ohmcomm::endl;
            continue;
        }
        std::string value;
        //3. read value, if any
        if(param->hasValue())
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
                ohmcomm::warn("Parameters") << "No value set for parameter \"" << param->longName << "\", skipping." << ohmcomm::endl;
                ohmcomm::warn("Parameters") << "See \"OHMComm --help\" for a list of parameters and their accepted values" << ohmcomm::endl;
                continue;
            }
        }
        //4. set mapping
        readParameters.push_back(ParameterValue(param, value));
        //4.1 if audio-processor, add to list
        if(param == AUDIO_PROCESSOR)
        {
            processorNames.push_back(value);
        }
    }
}


bool Parameters::parseParametersFromFile(const std::string& configFile)
{
    try
    {
        ohmcomm::info("Parameters") << "Reading configuration file... " << configFile << ohmcomm::endl;
        //read configuration-file
        std::fstream stream(configFile, std::fstream::in);
        stream.exceptions ( std::ios::badbit );
        std::string line;
        std::string::size_type index;
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
            
            const Parameter* param = nullptr;

            //read key
            index = line.find('=');
            if(index == std::string::npos)
            {
                ohmcomm::warn("Parameters") << "Invalid configuration line: " << line << ohmcomm::endl;
                continue;
            }
            key = Utility::trim(line.substr(0, index));
            for(const Parameter& p : availableParameters)
            {
                if(p.longName.compare(key) == 0)
                {
                    param = &p;
                    break;
                }
            }
            if(param == nullptr)
            {
                //Unrecognized argument - print message and continue
                ohmcomm::warn("Parameters") << "Parameter \"" << line << "\" not found, skipping." << ohmcomm::endl;
                ohmcomm::warn("Parameters") << "See \"OHMComm --help\" for a list of all parameters" << ohmcomm::endl;
                continue;
            }
            index++;
            //read value
            value = Utility::trim(line.substr(index));
            if(value[0] == '"')
            {
                value = value.substr(1, value.size()-2);
                //unescape escapes
                Utility::replaceAll(value, "\\\"", "\"");
            }
            else if(value.compare("true") == 0)
                value = "1";
            else if(value.compare("false") == 0)
                value = "0";
            //save value
            readParameters.push_back(ParameterValue(param, value));
            //4.1 if audio-processor, add to list
            if(param == AUDIO_PROCESSOR)
            {
                processorNames.push_back(value);
            }
        }
        stream.close();
    }
    catch(const std::ios_base::failure& f)
    {
        ohmcomm::error("Parameters") << "Failed to read configuration-file!" << ohmcomm::endl;
        ohmcomm::error("Parameters") << strerror(errno) << ohmcomm::endl;
        ohmcomm::error("Parameters") << f.what() << ohmcomm::endl;
        return false;
    }
    ohmcomm::info("Parameters") << "Configuration-file read" << ohmcomm::endl;
    return true;
}

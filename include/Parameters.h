/*
 * File:   Parameters.h
 * Author: daniel
 *
 * Created on July 22, 2015, 1:57 PM
 */

#ifndef PARAMETERS_H
#define	PARAMETERS_H

#include "configuration.h"

#include <string>
#include <vector>
#include <list>
#include <iostream>

enum class ParameterCategory : char
{
    //General configuration
    GENERAL,
    //Parameters setting audio-configuration
    AUDIO,
    //Any parameter setting a value for NetworkWrapper
    NETWORK,
    //Parameters which set the active processors or processor-specific values
    PROCESSORS,
    //Parameter to fill the SDES-values for this instance
    SOURCE_DESCRIPTION
};

struct Parameter
{
    //flag determining whether this parameter is required
    static constexpr unsigned short FLAG_REQUIRED = 0x1;
    //flag determining whether this parameter has a value
    static constexpr unsigned short FLAG_HAS_VALUE = 0x2;
    //flag determining whether this parameter calls another configuration-mode
    static constexpr unsigned short FLAG_CONFIGURATION_MODE = 0x4;
    
    //The category of this parameter
    const ParameterCategory category;
    //flags for the parameter
    const unsigned short flags;
    /*!
     * Short name for the parameter, a single character.
     * This character must be unique (case sensitive) within the list of available parameters
     */
    const char shortName;
    //Long name for the parameter, consists of whole word(s)
    const std::string longName;
    //An info-string about the values and usage of this parameter
    const std::string infoText;
    //The default parameter-value
    const std::string defaultValue;

    Parameter(ParameterCategory category, unsigned short flags, char shortName, std::string fullName,
        std::string infoText, std::string defValue) : category(category), flags(flags), shortName(shortName),
        longName(fullName), infoText(infoText), defaultValue(defValue)
    {
    }

    /*!
     * Constructor for a simple flag without any value
     */
    Parameter(ParameterCategory category, char shortName, std::string fullName, std::string infoText) :
        category(category), flags(0), shortName(shortName), longName(fullName), infoText(infoText),
        defaultValue("")
    {
    }

    /*!
     * Constructor for a parameter with value
     */
    Parameter(ParameterCategory category, char shortName, std::string fullName, std::string infoText,
        std::string defValue) : category(category), flags(FLAG_HAS_VALUE), shortName(shortName), longName(fullName),
        infoText(infoText), defaultValue(defValue)
    {
    }

    //We don't want multiple instances of the same parameter
    Parameter(const Parameter& other)  = delete;
    
    Parameter(Parameter&& other)  = default;
    
    inline bool isRequired() const
    {
        return (flags & FLAG_REQUIRED) == FLAG_REQUIRED;
    }
    
    inline bool hasValue() const
    {
        return (flags & FLAG_HAS_VALUE) == FLAG_HAS_VALUE;
    }
    
    inline bool isConfigurationMode() const
    {
        return (flags & FLAG_CONFIGURATION_MODE) == FLAG_CONFIGURATION_MODE;
    }
    
    inline bool hasDefaultValue() const
    {
        return defaultValue != "";
    }

    //overloads < operator -> for sorting
    inline bool operator <(const Parameter& other) const
    {
        return longName < other.longName;
    }
    
    inline bool operator >(const Parameter& other) const
    {
        return other.longName < longName;
    }
};


struct ParameterValue
{
    const Parameter* parameter;
    const std::string value;

    ParameterValue(const Parameter* parameter, const std::string value) :
        parameter(parameter), value(value)
    {
    }
};

/*!
 * Utility class to parse and interpret command-line parameters
 */
class Parameters
{
public:

    static const Parameter* HELP;
    static const Parameter* LIST_LOCAL_ADDRESSES;
    static const Parameter* LIST_AUDIO_DEVICES;
    static const Parameter* PASSIVE_CONFIGURATION;
    static const Parameter* WAIT_FOR_PASSIVE_CONFIG;
    static const Parameter* SIP_LOCAL_PORT;
    static const Parameter* SIP_REMOTE_PORT;
    static const Parameter* LOG_TO_FILE;
    static const Parameter* AUDIO_HANDLER;
    static const Parameter* INPUT_DEVICE;
    static const Parameter* OUTPUT_DEVICE;
    static const Parameter* FORCE_AUDIO_FORMAT;
    static const Parameter* FORCE_SAMPLE_RATE;
    static const Parameter* REMOTE_ADDRESS;
    static const Parameter* REMOTE_PORT;
    static const Parameter* LOCAL_PORT;
    static const Parameter* AUDIO_PROCESSOR;
    static const Parameter* PROFILE_PROCESSORS;
    static const Parameter* ENABLE_DTX;
    
    static const Parameter* SDES_CNAME;
    static const Parameter* SDES_EMAIL;
    static const Parameter* SDES_LOC;
    static const Parameter* SDES_NAME;
    static const Parameter* SDES_PHONE;
    static const Parameter* SDES_NOTE;

    /*!
     * This method can be used to add processor-specific parameters to the list of available (and parsed parameters).
     * For Parameters to be used in parameterized configuration-mode, this method must be called in a static context.
     * For that, it must be used to initialize a static constant value.
     * 
     * \param param The new parameter to add. This should be a newly created Parameter-object
     * 
     * \return A pointer to the registered parameter, nullptr otherwise
     */
    static const Parameter* registerParameter(Parameter&& param);
    
    /*!
     * \param paramName The long-name for the parameter to be returned
     * 
     * Returns a pointer to the parameter for the given name or a nullptr if no such parameter was registered
     */
    static const Parameter* getParameter(const std::string paramName);

    /*!
     * Default constructor accepting a list of all available audio-handlers and audio-processors
     *
     * \param availableHandlerNames The names of the audio-handlers to print. This is only required for printing the help-page
     * \param availableProcessorNames The names of the audio-processors to print. This is only required for printing the help-page
     */
    Parameters(const std::vector<std::string> availableHandlerNames, const std::vector<std::string> availableProcessorNames);

    /*!
     * Parses the arguments passed to main(argc,argv) if there are any
     *
     * \param argc The argument count
     * \param argv The arguments
     *
     * \return whether arguments have been set and parsed
     */
    bool parseParameters(int argc, char* argv[]);

    /*!
     * Prints the help-text to stdout
     *
     * \param allProcessorNames The names of all audio-processor to be printed
     */
    void printHelpPage() const;

    /*!
     * \return whether this specific parameter has been set
     */
    bool isParameterSet(const Parameter* param) const;

    /*!
     * NOTE: this method will return the default-value if the parameter has not been set explicitly
     *
     * \return the value for this parameter
     */
    const std::string getParameterValue(const Parameter* param) const;

    /*!
     * \return a vector with all configured audio-processor names
     */
    const std::vector<std::string> getAudioProcessors() const;

private:
    // A list of all available parameters
    //we use list for easier sorting
    static std::list<Parameter> availableParameters;
    static constexpr unsigned int tabSize{5};

    //Prints a help-line for a single parameter
    void printParameterHelp(const Parameter& param) const;

    std::vector<ParameterValue> readParameters;
    std::vector<std::string> processorNames;

    const std::vector<std::string> allAudioHandlerNames;
    const std::vector<std::string> allProcessorNames;
    
    /*!
     * Extracts the parameter from the command line arguments
     */
    void parseParametersCommandLine(int argc, char* argv[]);
    
    /*!
     * Reads the given file and retrieves parameters from it
     */
    bool parseParametersFromFile(const std::string& configFile);
};

#endif	/* PARAMETERS_H */


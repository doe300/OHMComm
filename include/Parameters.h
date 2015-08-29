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
#include <iostream>

enum class ParameterCategory
{
    //General configuration
    GENERAL,
    //Parameters setting audio-configuration
    AUDIO,
    //Any parameter setting a value for NetworkWrapper
    NETWORK,
    //Parameters which set the active processors or processor-specific values
    PROCESSORS
};

struct Parameter
{
    //The category of this parameter
    const ParameterCategory category;
    //Whether this parameter is required to be set
    const bool required;
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
    //Whether this parameter has a value
    const bool hasValue;

    Parameter(ParameterCategory category, bool required, char shortName, std::string fullName,
        std::string infoText, std::string defValue, bool hasValue) : category(category), required(required), shortName(shortName),
        longName(fullName), infoText(infoText), defaultValue(defValue), hasValue(hasValue)
    {
    }

    /*!
     * Constructor for a simple flag without any value
     */
    Parameter(ParameterCategory category, char shortName, std::string fullName, std::string infoText) :
        category(category), required(false), shortName(shortName), longName(fullName), infoText(infoText),
        defaultValue(""), hasValue(false)
    {
    }

    /*!
     * Constructor for a parameter with value
     */
    Parameter(ParameterCategory category, char shortName, std::string fullName, std::string infoText,
        std::string defValue) : category(category), required(false), shortName(shortName), longName(fullName),
        infoText(infoText), defaultValue(defValue), hasValue(true)
    {
    }

    //We don't want multiple instances of the same parameter
    Parameter(const Parameter& other)  = delete;

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

    static const Parameter HELP;
    static const Parameter PASSIVE_CONFIGURATION;
    static const Parameter LOG_TO_FILE;
    static const Parameter AUDIO_HANDLER;
    static const Parameter INPUT_DEVICE;
    static const Parameter OUTPUT_DEVICE;
    static const Parameter FORCE_AUDIO_FORMAT;
    static const Parameter FORCE_SAMPLE_RATE;
    static const Parameter REMOTE_ADDRESS;
    static const Parameter REMOTE_PORT;
    static const Parameter LOCAL_PORT;
    static const Parameter AUDIO_PROCESSOR;
    static const Parameter PROFILE_PROCESSORS;

    // A list of all available parameters
    static const std::vector<const Parameter*> availableParameters;

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
     * \param allProcessorNames The names of the audio-processors to print. This is only required for printing the help-page
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
    bool isParameterSet(const Parameter& param) const;

    /*!
     * NOTE: this method will return the default-value if the parameter has not been set explicitly
     *
     * \return the value for this parameter
     */
    const std::string getParameterValue(const Parameter& param) const;

    /*!
     * \return a vector with all configured audio-processor names
     */
    const std::vector<std::string> getAudioProcessors() const;

private:
    static const unsigned int tabSize{5};
    //Returns the name of the given category
    std::string getCategoryName(const ParameterCategory& category) const;

    //Prints a help-line for a single parameter
    void printParameterHelp(const Parameter* param) const;

    std::vector<ParameterValue> readParameters;
    std::vector<std::string> processorNames;

    const std::vector<std::string> allAudioHandlerNames;
    const std::vector<std::string> allProcessorNames;
};

#endif	/* PARAMETERS_H */


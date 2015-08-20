/*
 * File:   ConfigurationMode.h
 * Author: daniel
 *
 * Created on August 19, 2015, 4:48 PM
 */

#ifndef CONFIGURATIONMODE_H
#define	CONFIGURATIONMODE_H

#include <vector>
#include <stdexcept>

#include "configuration.h"
#include "UserInput.h"
#include "Parameters.h"
#include "AudioHandlerFactory.h"

/*!
 * Abstract super-class for the various kinds of configuration-modes
 */
class ConfigurationMode
{
public:
    ConfigurationMode();
    virtual ~ConfigurationMode();

    /*!
     * Runs the configuration, if possible.
     *
     * Any successive calls to this method will be no-ops
     *
     * \return Whether the configuration was successfully run
     */
    virtual bool runConfiguration() = 0;

    /*!
     * \return Whether this configuration if fully configured
     */
    virtual bool isConfigured() const;

    /*!
     * \return a pair consisting of (first) the name for the audio-handler to use and (second) a flag whether to use the manual audio-configuration
     */
    const std::pair<std::string, bool> getAudioHandlerConfiguration() const;

    /*!
     * \return the configured AudioConfiguration
     */
    const AudioConfiguration getAudioConfiguration() const;

    /*!
     * \return the configured NetworkConfiguration
     */
    const NetworkConfiguration getNetworkConfiguration() const;

    /*!
     * \return a pair containing (first) all configured processor-names and (second) a flag whether to profile the processors
     */
    bool getAudioProcessorsConfiguration(std::vector<std::string>& processorNames) const;

    /*!
     * \return a pair containing (first) a flag whether to write a log to file and (second) the file-name for the log-file
     */
    const std::pair<bool, std::string> getLogToFileConfiguration() const;

protected:

    void createDefaultNetworkConfiguration();

    bool isConfigurationDone = false;
    bool useDefaultAudioConfig = true;
    std::string audioHandlerName;
    AudioConfiguration audioConfig{0};
    NetworkConfiguration networkConfig{0};
    std::vector<std::string> processorNames;
    bool profileProcessors = false;
    bool logToFile = false;
    std::string logFileName;
};

/*!
 * Configuration using the given Parameters
 */
class ParameterConfiguration : public ConfigurationMode
{
public:
    ParameterConfiguration(const Parameters& params);

    virtual bool runConfiguration();

private:
    void fillAudioConfiguration(int outputDeviceID, int inputDeviceID);
};

/*!
 * Configuration utilizing UserInput to ask the user for settings
 */
class InteractiveConfiguration : public ConfigurationMode
{
public:
    InteractiveConfiguration()
    {
    }

    virtual bool runConfiguration();

private:

    void interactivelyConfigureAudioDevices();
    void interactivelyConfigureNetwork();
    void interactivelyConfigureProcessors();
};

/*!
 * Configuration via the provided methods to be used as library
 */
class LibraryConfiguration : public ConfigurationMode
{
public:
    LibraryConfiguration()
    {
        //initialize configuration with default values as far as possible
        audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
        createDefaultNetworkConfiguration();
    }

    virtual bool runConfiguration();

    virtual bool isConfigured() const;

    /*!
     * Configures the audio-handler for the LIBRARY configuration-mode
     *
     * \param audioHandlerName The name of the AudioHandler to use
     *
     * \param audioConfig The audio-configuration or a nullptr
     *
     */
    void configureAudio(const std::string audioHandlerName, const AudioConfiguration* audioConfig);

    /*!
     * Configures the network for the LIBRARY configuration-mode
     *
     * \param networkConfig The network-configuration to use
     */
    void configureNetwork(const NetworkConfiguration& networkConfig);

    /*!
     * Configures the audio-processors for the LIBRARY configuration-mode
     *
     * \param processorNames A list of names of audio-processors to add (in the given order)
     *
     * \param profileProcessors Whether to create profiler for the given processors
     */
    void configureProcessors(const std::vector<std::string>& processorNames, bool profileProcessors);

    /*!
     * (Optional) Configures the log-file for the LIBRARY configuration-mode
     *
     * \param logFileName The file-name of the log-file
     *
     */
    void configureLogToFile(const std::string logFileName);

private:
    bool isAudioConfigured = false;
    bool isNetworkConfigured = false;
    bool isProcessorsConfigured = false;
};

/*!
 * Passive configuration, the application will ask its communication-partner for its audio-configuration
 */
class PassiveConfiguration : public ConfigurationMode
{
public:
    PassiveConfiguration(const NetworkConfiguration& networkConfig);

    virtual bool runConfiguration();
};
#endif	/* CONFIGURATIONMODE_H */


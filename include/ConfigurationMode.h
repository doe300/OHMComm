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
#include <map>
#include <stdlib.h>

#include "configuration.h"
#include "Parameters.h"

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
    
    /*!
     * \return whether the program will wait on startup for the communication partner to request a passive configuration before starting audio-playback
     */
    const bool isWaitForConfigurationRequest() const;

    /*!
     * Configuration-mode specific method to retrieve a value for the given key
     *
     * \param key The name of the configuration-value
     * \param message An optional message to display (e.g. for interactive configuration)
     * \param defaultValue The default value to use
     *
     * \return the value for this configuration-key, defaults to the defaultValue
     */
    virtual const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const = 0;

    /*!
     * Configuration-mode specific method to retrieve a value for the given key
     *
     * \param key The name of the configuration-value
     * \param message An optional message to display (e.g. for interactive configuration)
     * \param defaultValue The default value to use
     *
     * \return the value for this configuration-key, defaults to the defaultValue
     */
    virtual const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const = 0;

    /*!
     * Configuration-mode specific method to retrieve a value for the given key
     *
     * \param key The name of the configuration-value
     * \param message An optional message to display (e.g. for interactive configuration)
     * \param defaultValue The default value to use
     *
     * \return the value for this configuration-key, defaults to the defaultValue
     */
    virtual const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const = 0;
    
    /*!
     * Configuration-mode specific method to check whether a configuration-value is set.
     * Implementations are allowed to ask the user whether this key should be set.
     * 
     * \param key The name of the configuration-value
     * \param message An optional message to display (e.g. for interactive configuration)
     * 
     * \return whether this configuration-key is set
     */
    virtual const bool isCustomConfigurationSet(const std::string key, const std::string message) const = 0;

    /*!
     * This method is called by OHMComm to update the audio-configuration in this ConfigurationMode with the actual used configuration
     */
    void updateAudioConfiguration(const AudioConfiguration& audioConfig);
    
protected:

    void createDefaultNetworkConfiguration();

    bool waitForConfigurationRequest = false;
    bool isConfigurationDone = false;
    bool useDefaultAudioConfig = true;
    std::string audioHandlerName;
    AudioConfiguration audioConfig; // Initialization in constructor
    NetworkConfiguration networkConfig; // Initialization in constructor
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
    const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
    const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    const bool isCustomConfigurationSet(const std::string key, const std::string message) const;

private:
    const Parameters params;
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
    const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
    const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    const bool isCustomConfigurationSet(const std::string key, const std::string message) const;

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
    LibraryConfiguration();

    virtual bool runConfiguration();

    virtual bool isConfigured() const;
    const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
    const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    const bool isCustomConfigurationSet(const std::string key, const std::string message) const;

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
    
    /*!
     * (Optional) Configures whether this instance will wait for a passive configuration-request
     * 
     * \param waitForConfig Whether to wait for passive configuration-request
     */
    void configureWaitForConfigurationRequest(bool waitForConfig);

    void configureCustomValue(std::string key, std::string value);
    void configureCustomValue(std::string key, int value);
    void configureCustomValue(std::string key, bool value);

private:
    std::map<std::string, std::string> customConfig;
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

    /*! The size of a configuration-message (without the vector) in bytes */
    static const uint8_t CONFIGURATION_MESSAGE_SIZE{12};

    struct ConfigurationMessage
    {
        unsigned int sampleRate;            // 4 bytes
        unsigned int audioFormat;           // 4 bytes
        unsigned short bufferFrames;        // 2 bytes
        unsigned int nChannels : 8;         // 1 byte
        unsigned int numProcessorNames : 8; // 1 bytes
        std::vector<std::string> processorNames;
        
        ConfigurationMessage() : sampleRate(0), audioFormat(0), bufferFrames(0), nChannels(0), numProcessorNames(0), processorNames({})
        {
            
        }
    };

    PassiveConfiguration(const NetworkConfiguration& networkConfig, bool profileProcessors = false, std::string logFile = "");

    virtual bool runConfiguration();
    const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
    const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    const bool isCustomConfigurationSet(const std::string key, const std::string message) const;

    static const ConfigurationMessage readConfigurationMessage(void* buffer, unsigned int bufferSize);

    static unsigned int writeConfigurationMessage(void* buffer, unsigned int maxBufferSize, ConfigurationMessage& configMessage);
};

/*!
 * File-based configuration.
 *
 * The format for the configuration-file is following:
 *
 * - Lines starting with a '#' are ignored as comments
 * - Every other non-empty line is interpreted as key-value pair
 * - the key is an arbitrary string of digits (0-9), letters (a-z, A-Z), underscore (_) and minus (-)
 *      Note: any other character may occur, but interpretation is undefined
 * - the value is one of the following:
 *      - a number (as understood by atoi())
 *      - a boolean value ('true' or 'false'). Booleans can also be represented as numbers 0 for false, any other number for true
 *      - a string (arbitrary list of characters enclosed by double-quotes '"')
 *
 * The names of the predefined configuration-keys are taken from the existing Parameter-constants in Parameters
 */
class FileConfiguration : public ConfigurationMode
{
public:
    FileConfiguration(const std::string fileName);

    virtual bool runConfiguration();

    const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
    const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    const bool isCustomConfigurationSet(const std::string key, const std::string message) const;

private:
    const std::string configFile;
    std::map<std::string, std::string> customConfig;
};
#endif	/* CONFIGURATIONMODE_H */


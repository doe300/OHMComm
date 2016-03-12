/*
 * File:   ConfigurationMode.h
 * Author: daniel
 *
 * Created on August 19, 2015, 4:48 PM
 */

#ifndef CONFIGURATIONMODE_H
#define	CONFIGURATIONMODE_H

#include <vector>
#include <algorithm>
#include <stdexcept>

#include "configuration.h"
#include "PlaybackListener.h"

namespace ohmcomm
{

    /*!
     * Abstract super-class for the various kinds of configuration-modes
     */
    class ConfigurationMode : public PlaybackListener
    {
        //TODO split configuration into audio-config and local-config (everything not of interest to the remote device)
        //or at least make sure, all configuration-modes can configure both (SIP/passive e.g. can't configure local config, e.g gain, profiling, ...)
        //-> fall back to parameters/any other mode? (would need to retrieve values without being fully configured)

        //TODO allow for configuration of buffer-delay or make adaptive

        //TODO cache values for keys for all modes, so they can be reused
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
         * \return the configured NetworkConfiguration for the RTCP-port
         */
        virtual const NetworkConfiguration getRTCPNetworkConfiguration() const;

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
        bool isWaitForConfigurationRequest() const;

        /*!
         * A payload-type of -1 lets the audio-processors decide the type
         * 
         * \return the configured payload-type to be used
         */
        short getPayloadType() const;

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
         * Convenience-wrapper for #getCustomConfiguration(const std::string, const std::string, const std::string)
         */
        const std::string getCustomConfiguration(const std::string key, const std::string message, const char* defaultValue) const
        {
            return getCustomConfiguration(key, message, std::string(defaultValue));
        }

        /*!
         * Configuration-mode specific method to retrieve a value for the given key
         *
         * \param key The name of the configuration-value
         * \param message An optional message to display (e.g. for interactive configuration)
         * \param defaultValue The default value to use
         *
         * \return the value for this configuration-key, defaults to the defaultValue
         */
        virtual int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const = 0;

        /*!
         * Configuration-mode specific method to retrieve a value for the given key
         *
         * \param key The name of the configuration-value
         * \param message An optional message to display (e.g. for interactive configuration)
         * \param defaultValue The default value to use
         *
         * \return the value for this configuration-key, defaults to the defaultValue
         */
        virtual bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const = 0;

        /*!
         * Configuration-mode specific method to check whether a configuration-value is set.
         * Implementations are allowed to ask the user whether this key should be set.
         * 
         * \param key The name of the configuration-value
         * \param message An optional message to display (e.g. for interactive configuration)
         * 
         * \return whether this configuration-key is set
         */
        virtual bool isCustomConfigurationSet(const std::string key, const std::string message) const = 0;

        /*!
         * This method is called by OHMComm to update the audio-configuration in this ConfigurationMode with the actual used configuration
         */
        void updateAudioConfiguration(const AudioConfiguration& audioConfig);

    protected:

        void createDefaultNetworkConfiguration();

        short payloadType = -1;
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
}
#endif	/* CONFIGURATIONMODE_H */


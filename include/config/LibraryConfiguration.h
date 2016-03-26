/* 
 * File:   LibraryConfiguration.h
 * Author: daniel
 *
 * Created on November 30, 2015, 5:34 PM
 */

#ifndef LIBRARYCONFIGURATION_H
#define	LIBRARYCONFIGURATION_H

#include "ConfigurationMode.h"
#include <map>

namespace ohmcomm
{

    /*!
     * Configuration via the provided methods to be used as library
     */
    class LibraryConfiguration : public ConfigurationMode
    {
    public:
        LibraryConfiguration();

        virtual bool runConfiguration() override;

        virtual bool isConfigured() const override;
        const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const override;
        int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const override;
        bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const override;
        bool isCustomConfigurationSet(const std::string key, const std::string message) const override;

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
}
#endif	/* LIBRARYCONFIGURATION_H */


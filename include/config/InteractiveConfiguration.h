/* 
 * File:   InteractiveConfiguration.h
 * Author: daniel
 *
 * Created on November 30, 2015, 5:33 PM
 */

#ifndef INTERACTIVECONFIGURATION_H
#define	INTERACTIVECONFIGURATION_H

#include "ConfigurationMode.h"
#include "UserInput.h"
#include "audio/AudioHandler.h"

namespace ohmcomm
{

    /*!
     * Configuration utilizing UserInput to ask the user for settings
     */
    class InteractiveConfiguration : public ConfigurationMode
    {
    public:

        InteractiveConfiguration()
        {
        }

        virtual bool runConfiguration() override;
        const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const override;
        int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const override;
        bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const override;
        bool isCustomConfigurationSet(const std::string key, const std::string message) const override;

    private:

        void interactivelyConfigureAudioDevices(std::unique_ptr<AudioHandler>&& handler);
        void interactivelyConfigureNetwork();
        void interactivelyConfigureProcessors();
    };
}
#endif	/* INTERACTIVECONFIGURATION_H */


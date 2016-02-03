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
    int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    bool isCustomConfigurationSet(const std::string key, const std::string message) const;

private:

    void interactivelyConfigureAudioDevices();
    void interactivelyConfigureNetwork();
    void interactivelyConfigureProcessors();
};
#endif	/* INTERACTIVECONFIGURATION_H */


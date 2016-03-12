/* 
 * File:   ParameterConfiguration.h
 * Author: daniel
 *
 * Created on November 30, 2015, 5:32 PM
 */

#ifndef PARAMETERCONFIGURATION_H
#define	PARAMETERCONFIGURATION_H

#include "ConfigurationMode.h"
#include "Parameters.h"
namespace ohmcomm
{

    /*!
     * Configuration using the given Parameters.
     * 
     * This can also be used as fallback configuration-mode since it doesn't require to be fully configured before returning values
     */
    class ParameterConfiguration : public ConfigurationMode
    {
    public:
        ParameterConfiguration(const Parameters& params);

        virtual bool runConfiguration();
        const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
        int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
        bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
        bool isCustomConfigurationSet(const std::string key, const std::string message) const;

    private:
        const Parameters params;
        void fillAudioConfiguration(int outputDeviceID, int inputDeviceID);
    };
}
#endif	/* PARAMETERCONFIGURATION_H */


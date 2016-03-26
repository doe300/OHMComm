/* 
 * File:   PassiveConfiguration.h
 * Author: daniel
 *
 * Created on November 30, 2015, 5:35 PM
 */

#ifndef PASSIVECONFIGURATION_H
#define	PASSIVECONFIGURATION_H

#include "ConfigurationMode.h"
#include "rtp/RTCPPackageHandler.h"
#include "network/UDPWrapper.h"

namespace ohmcomm
{

    /*!
     * Passive configuration, the application will ask its communication-partner for its audio-configuration
     */
    class PassiveConfiguration : public ConfigurationMode
    {
    public:

        /*! The size of a configuration-message (without the vector) in bytes */
        static const uint8_t CONFIGURATION_MESSAGE_SIZE{12};

        struct ConfigurationMessage //TODO network byte-order
        {
            unsigned int sampleRate; // 4 bytes
            unsigned int audioFormat; // 4 bytes
            unsigned short bufferFrames; // 2 bytes
            unsigned int nChannels : 8; // 1 byte
            unsigned int numProcessorNames : 8; // 1 bytes
            std::vector<std::string> processorNames;

            ConfigurationMessage() : sampleRate(0), audioFormat(0), bufferFrames(0), nChannels(0), numProcessorNames(0), processorNames({})
            {

            }
        };

        PassiveConfiguration(const NetworkConfiguration& networkConfig, bool profileProcessors = false, std::string logFile = "");

        virtual bool runConfiguration() override;
        const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const override;
        int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const override;
        bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const override;
        bool isCustomConfigurationSet(const std::string key, const std::string message) const override;

        static const ConfigurationMessage readConfigurationMessage(const void* buffer, unsigned int bufferSize);

        static int writeConfigurationMessage(void* buffer, unsigned int maxBufferSize, ConfigurationMessage& configMessage);
    };
}
#endif	/* PASSIVECONFIGURATION_H */


/* 
 * File:   SIPConfiguration.h
 * Author: daniel
 *
 * Created on December 2, 2015, 1:06 PM
 */

#ifndef SIPCONFIGURATION_H
#define	SIPCONFIGURATION_H

#include "config/ParameterConfiguration.h"
#include "SIPHandler.h"

namespace ohmcomm
{
    namespace sip
    {

        class SIPConfiguration : public ParameterConfiguration
        {
        public:
            SIPConfiguration(const Parameters& params, const NetworkConfiguration& sipConfig);

            ~SIPConfiguration();

            bool runConfiguration() override;

            const NetworkConfiguration getRTCPNetworkConfiguration() const override;

            const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const override;
            int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const override;
            bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const override;
            bool isCustomConfigurationSet(const std::string key, const std::string message) const override;


            void onRegister(PlaybackObservee* ohmComm) override;
            void onPlaybackStop() override;

        private:
            SIPHandler handler;
            NetworkConfiguration rtcpConfig;
            std::map<std::string, std::string> customConfig;

            /*!
             * Called by the SIPHandler to set the network- and audio-configuration
             */
            void setConfig(const MediaDescription& media, const NetworkConfiguration& rtpConfig, const NetworkConfiguration& customRTCPConfig);

            //Wait a maximum of 30 secs
            static constexpr int MAX_WAIT_TIME{30000};
        };
    }
}
#endif	/* SIPCONFIGURATION_H */


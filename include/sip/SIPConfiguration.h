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

            bool runConfiguration();

            const NetworkConfiguration getRTCPNetworkConfiguration() const;

            const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
            int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
            bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
            bool isCustomConfigurationSet(const std::string key, const std::string message) const;


            void onRegister(PlaybackObservee* ohmComm);
            void onPlaybackStop();

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


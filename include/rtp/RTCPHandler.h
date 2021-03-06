/* 
 * File:   RTCPHandler.h
 * Author: daniel
 *
 * Created on November 5, 2015, 11:04 AM
 */

#ifndef RTCPHANDLER_H
#define	RTCPHANDLER_H

#include <memory>
#include <thread>
#include "network/NetworkWrapper.h"
#include "ParticipantDatabase.h"
#include "RTCPPackageHandler.h"
#include "config/ConfigurationMode.h"

namespace ohmcomm
{
    namespace rtp
    {

        /*!
         * The RTCPHandler manages a thread for RTCP-communication.
         * 
         * The port occupied by RTCP is per standard the RTP-port +1.
         * 
         * The RTCP handler is managed by the RTP-processor
         */
        class RTCPHandler : private ParticipantListener
        {
        public:
            RTCPHandler(const NetworkConfiguration& rtcpConfig, const std::shared_ptr<ConfigurationMode> configMode, const bool isActiveSender = true);
            ~RTCPHandler();

            virtual void onRemoteAdded(const unsigned int ssrc) override;

            virtual void onRemoteRemoved(const unsigned int ssrc) override;

        private:
            const std::unique_ptr<ohmcomm::network::NetworkWrapper> wrapper;
            const std::shared_ptr<ConfigurationMode> configMode;
            const bool isActiveSender;
            RTCPPackageHandler rtcpHandler;
            Participant& ourselves;

            std::thread listenerThread;
            bool threadRunning = false;

            //send SR/RR every X seconds
            static const std::chrono::seconds sendSRInterval;
            //timeout before the remote is considered offline
            static const std::chrono::seconds remoteDropoutTimeout;

            /*!
             * Method called in the parallel thread, receiving RTCP-packages and handling them
             */
            void runThread();
            
             /*!
             * Shuts down the RTCP-thread
             */
            void shutdown();

            /*!
             * Starts the RTCP-thread
             */
            void startUp();

            /*!
             * This method handles received RTCP packages and is called only from #runThread()
             * 
             * \return the SSRC of the originating participant
             */
            uint32_t handleRTCPPackage(const void* receiveBuffer, unsigned int receivedSize);

            void shutdownInternal();

            /*!
             * Sends a Source Description-package, only called from #runThread()
             */
            void sendSourceDescription();

            /*!
             * Sends a Source BYE-package, only called from #runThread()
             */
            void sendByePackage();

            /*!
             * Creates a SR package to be used in a compound package
             */
            const void* createSenderReport(unsigned int offset = 0);

            /*!
             * Creates a RR package to be used in a compound package
             */
            const void* createReceiverReport(unsigned int offset = 0);

            /*!
             * Creates reception reports for all remote participants
             */
            static const std::vector<ReceptionReport> createReceptionReports();

            /*!
             * Creates a SDES package to be used in a compound package
             */
            const void* createSourceDescription(unsigned int offset = 0);

            static void printReceptionReports(const std::vector<ReceptionReport>& reports);
            
            //enables access to private methods for RTP-processor
            friend class ProcessorRTP;
        };
    }
}
#endif	/* RTCPHANDLER_H */


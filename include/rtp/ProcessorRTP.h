#ifndef PROCESSORRTP_H
#define	PROCESSORRTP_H

#include "ParticipantDatabase.h"
#include "processors/AudioProcessor.h"
#include "RTPPackageHandler.h"
#include "network/NetworkWrapper.h"
#include "RTPBufferHandler.h"
#include "RTPListener.h"
#include "JitterBuffers.h"

namespace ohmcomm
{
    namespace rtp
    {

        /*!
         * AudioProcessor wrapping/unwrapping audio-frames in/out of a RTP-package
         *
         * This implementation uses a separate thread (see RTPListener) to receive packages.
         * This was implemented to avoid blocking the audio-loop while waiting for packages.
         * The received RTPPackages are buffered in an RTPBuffer
         */
        class ProcessorRTP : public AudioProcessor
        {
        public:
            /*!
             * Constructs a new ProcessorRTP
             *
             * \param name The name of the AudioProcessor
             *
             * \param networkConfig The network-configuration to use sending packages
             *
             * \param payloadType The payload-type for the RTP packages
             */
            ProcessorRTP(const std::string name, const NetworkConfiguration& networkConfig, const PayloadType payloadType);

            void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

            void startup();

            unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
            unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);

            bool cleanUp();
        private:
            //! Delay until treating input as silence
            //!Treat as silence after 500ms of no input
            static constexpr unsigned short SILENCE_DELAY{500};
            const std::shared_ptr<NetworkWrapper> network;
            JitterBuffers buffers;
            Participant& ourselves;
            std::unique_ptr<RTPPackageHandler> rtpPackage;
            std::unique_ptr<RTPListener> rtpListener;
            bool isDTXEnabled;
            bool lastPackageWasSilent;
            unsigned short totalSilenceDelayPackages;
            unsigned int currentSilenceDelayPackages;

            void initPackageHandler(unsigned int maxBufferSize);
        };
    }
}
#endif

/*
 * File:   OHMComm.h
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#ifndef OHMCOMM_H
#define	OHMCOMM_H

#include "PlaybackListener.h"

#include "config/ConfigurationMode.h"
#include "audio/AudioHandler.h"
#include "rtp/ParticipantDatabase.h"

#include <functional>
#include <memory> // unique_ptr
#include <string>
#include <stdexcept>
#include <vector>

namespace ohmcomm
{

    /*!
     * Main class containing all required methods to start and control the P2P communication.
     *
     * There are several modes, the OHMComm can run in. For a full list, see the ConfigurationMode subclasses
     */
    class OHMComm : public PlaybackObservee, private rtp::ParticipantListener
    {
    public:

        /*!
         * Creates a new OHMComm with the given configuration-mode
         */
        OHMComm(ConfigurationMode* mode);

        /*!
         * Delete copy-constructor
         */
        OHMComm(const OHMComm& orig) = delete;

        /*!
         * Delete copy-assignment
         */
        OHMComm& operator=(const OHMComm& orig) = delete;
        virtual ~OHMComm();

        /*!
         * \return The configuration mode of this OHMComm object
         */
        std::shared_ptr<ConfigurationMode> getConfigurationMode() const;

        /*!
         * Once this OHMComm-object has been started, all calls to any configuration will not change any configuration-values
         *
         * \return Whether the configuration is active
         */
        bool isConfigurationActive() const;

        /*!
         * \param runConfiguration Whether to run configuration if not yet configured
         *
         * \return Whether all necessary fields have been configured
         */
        bool isConfigurationDone(bool runConfiguration);

        /*!
         * Initializes and starts all audio-threads
         */
        void startAudioThreads();

        /*!
         * Stops all audio-threads
         */
        void stopAudioThreads();

        /*!
         * \return Whether the audio-communication is running
         */
        bool isRunning() const;
        
        void onRemoteRemoved(const unsigned int ssrc) override;

        //XXX remove this
        virtual std::function<void () > createStopCallback() override;

    private:

        /*! Overall mode of configuration */
        const std::shared_ptr<ConfigurationMode> configurationMode;
        /*! Flag, whether this object can be configured */
        bool configurationActive = true;
        /*! Flag, whether this OHMComm is currently being executed */
        bool running = false;

        //Configuration fields
        std::unique_ptr<AudioHandler> audioHandler;

        void configureRTPProcessor(bool profileProcessors, const PayloadType payloadType);

        void startAudio();
    };
}
#endif	/* OHMCOMM_H */


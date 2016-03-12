#ifndef AUDIOIO_H
#define	AUDIOIO_H

#include "AudioDevice.h"
#include "processors/ProcessorManager.h"
#include "configuration.h"
#include "config/ConfigurationMode.h"
#include <vector>
#include <memory> //for std::shared_ptr

namespace ohmcomm
{
    /*!
     * Base class for Audio framework.
     *
     * Implementations of this class wrap a specific audio library
     */
    class AudioHandler
    {
    public:

        //Container for modes of playback

        enum PlaybackMode : unsigned char
        {
            //mode is not yet determined
            UNDEFINED = 0x00,
            //audio-output is played
            OUTPUT = 0x01,
            //audio-input is recorded
            INPUT = 0x02,
            //input is recorded and output is played
            DUPLEX = 0x03
        };

        AudioHandler();

        virtual ~AudioHandler();

        /*!
         * Configures the AudioHandler
         *
         * \param audioConfiguration The Object which contains the configuration
         */
        virtual void setConfiguration(const AudioConfiguration &audioConfiguration) = 0;

        /*!
         * Starts processing audio according to the given mode.
         */
        void start(const PlaybackMode mode = DUPLEX);

        /*!
         * Suspends any processing
         */
        virtual void suspend() = 0;

        /*!
         * Resumes a suspended state
         */
        virtual void resume() = 0;

        /*!
         * Stops the communication process at all. The object cannot be resumed (Should be used on shutdown)
         */
        virtual void stop() = 0;

        /*!
         * Stops and resets the audio configuration (Instance can be reused with a new audio configuration)
         */
        virtual void reset() = 0;

        /*!
         * Loads a default audio configuration
         */
        virtual void setDefaultAudioConfig() = 0;

        /*!
         * Initializes the audio library and prepares for execution.
         *
         * This includes allocating memory, configuring the audio-processors or the audio-library, etc.
         */
        virtual bool prepare(const std::shared_ptr<ConfigurationMode> configMode) = 0;

        /*!
         * \return the audio-processor manager
         */
        ProcessorManager& getProcessors();

        /*!
         * Returns the current AudioConfiguration
         *
         * \return Current AudioConfiguration
         */
        AudioConfiguration getAudioConfiguration();

        /*!
         * Returns whether the instance has an AudioConfiguration
         *
         * \return Status of the AudioConfiguration
         */
        bool isAudioConfigSet() const;

        /*!
         * Returns whether instance is prepared (ready) for execution
         *
         * \return Prepared status of the instance
         */
        bool isPrepared() const;

        /*!
         * Returns a platform- and library-independent list of all available audio-devices
         * 
         * \return a reference to all local audio-devices
         */
        virtual const std::vector<AudioDevice>& getAudioDevices() = 0;

        /*!
         * This method can be used to determine, whether full duplex is currently supported
         * 
         * \return the currently configured playback mode
         */
        PlaybackMode getMode();

    protected:
        bool flagAudioConfigSet = false;
        bool flagPrepared = false;

        ProcessorManager processors;
        AudioConfiguration audioConfiguration;

        virtual void startHandler(const PlaybackMode mode) = 0;
    };
}
#endif

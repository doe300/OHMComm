/*
 * File:   OHMComm.h
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#ifndef OHMCOMM_H
#define	OHMCOMM_H

#include "ConfigurationMode.h"
#include "AudioHandler.h"
#include "RTPBuffer.h"
#include "NetworkWrapper.h"
#include "RTPListener.h"

#include <functional>
#include <memory> // unique_ptr
#include <string>
#include <stdexcept>
#include <vector>

/*!
 * Main class containing all required methods to start and control the P2P communication.
 *
 * There are 3 modes, the OHMComm can run in:
 *
 * 1. parameterized: all necessary configurations are passed via the second constructor
 * 2. programmatically: the configurations have to be set via the corresponding configure-methods
 * 3. interactive: upon start, the OHMComm will ask the user for the configuration via stdout/stdin
 */
class OHMComm
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
    ConfigurationMode* getConfigurationMode() const;

    /*!
     * Once this OHMComm-object has been started, all calls to any configuration will not change any configuration-values
     *
     * \return Whether the configuration is active
     */
    const bool isConfigurationActive() const;

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

private:

    std::shared_ptr<RTPBuffer> rtpBuffer;
    /*! Overall mode of configuration */
    ConfigurationMode* configurationMode;
    /*! Flag, whether this object can be configured */
    bool configurationActive = true;
    /*! Flag, whether this OHMComm is currently being executed */
    bool running = false;

    //Configuration fields
    std::unique_ptr<AudioHandler> audioHandler;
    std::shared_ptr<NetworkWrapper> networkWrapper;
    std::unique_ptr<RTPListener> listener;

    void configureRTPProcessor(bool profileProcessors);
    std::function<void ()> createStopCallback();
};

#endif	/* OHMCOMM_H */


/*
 * File:   OHMComm.h
 * Author: daniel
 *
 * Created on August 17, 2015, 4:42 PM
 */

#ifndef OHMCOMM_H
#define	OHMCOMM_H

#include "AudioHandler.h"
#include "Parameters.h"
#include "RTPBuffer.h"
#include "NetworkWrapper.h"
#include "RTPListener.h"

#include <functional>
#include <memory> // unique_ptr
#include <string>
#include <stdexcept>
#include <vector>

/*!
 * Mode the OHMComm object is run in
 */
enum class ConfigurationMode
{
    /*!
     * OHMComm will ask the user for configuration via stdout/stdin
     */
    INTERACTIVE,
    /*!
     * OHMComm will use the Parameters provided via the appropriate constructor
     */
    PARAMETERIZED,
    /*!
     * The caller can configure the OHMComm-object via its configuration-methods
     */
    LIBRARY
};

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
     * Creates a new OHMComm in the given mode (INTERACTIVE or LIBRARY)
     */
    OHMComm(const ConfigurationMode mode);

    /*!
     * Creates a new OHMComm in PARAMETERIZED mode
     */
    OHMComm(const Parameters params);

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
    const ConfigurationMode getConfigurationMode();

    /*!
     * All configureXYZ-methods can only be called as long as the configuration is active.
     * Once this OHMComm-object has been started, all calls to any configuration-method will throw an exception
     *
     * \return Whether the configuration is active
     */
    const bool isConfigurationActive();

    /*!
     * \param runInteractiveConfiguration Whether to run interactive configuration if used and not yet configured
     *
     * \return Whether all necessary fields have been configured
     */
    bool isConfigurationDone(bool runInteractiveConfiguration);

    /*!
     * Runs the interactive configuration
     */
    void configureInteractive();

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
    bool isRunning();

    /*!
     * Configures the audio-handler for the LIBRARY configuration-mode
     *
     * \param audioHandlerName The name of the AudioHandler to use
     *
     * \param audioConfig The audio-configuration
     *
     */
    void configureAudio(const std::string audioHandlerName, const AudioConfiguration& audioConfig);

    /*!
     * Configures the network for the LIBRARY configuration-mode
     *
     * \param networkConfig The network-configuration to use
     */
    void configureNetwork(const NetworkConfiguration& networkConfig);

    /*!
     * Configures the audio-processors for the LIBRARY configuration-mode
     *
     * \param processorNames A list of names of audio-processors to add (in the given order)
     *
     * \param profileProcessors Whether to create profiler for the given processors
     */
    void configureProcessors(const std::vector<std::string>& processorNames, bool profileProcessors);

    /*!
     * (Optional) Configures the log-file for the LIBRARY configuration-mode
     *
     * \param logFileName The file-name of the log-file
     *
     */
    void configureLogToFile(const std::string logFileName);

private:

    std::shared_ptr<RTPBuffer> rtpBuffer;
    /*! Overall mode of configuration */
    const ConfigurationMode configurationMode;
    /*! Flag, whether this object can be configured */
    bool configurationActive = true;
    /*! Flag, whether this OHMComm is currently being executed */
    bool running = false;

    //Configuration fields
    bool isAudioConfigured = false;
    std::unique_ptr<AudioHandler> audioHandler;
    bool isNetworkConfigured = false;
    std::shared_ptr<NetworkWrapper> networkWrapper;
    bool isProcessorsConfigured = false;
    bool profileProcessors = false;
    bool logStatisticsToFile = false;
    std::string logFileName{0};
    std::unique_ptr<RTPListener> listener;

    /*!
     * Checks whether this object is currently configurable
     *
     * NOTE: This method throws errors, if the configuration can't be performed
     */
    void checkConfigurable();

    /*!
     * Fills the audioConfig with the default-values for the given devices
     *
     * \param outputDeviceID The ID of the output-device
     * \param inputDeviceID The ID of the input-device
     */
    static AudioConfiguration fillAudioConfiguration(int outputDeviceID, int inputDeviceID);

    static AudioConfiguration interactivelyConfigureAudioDevices();
    static NetworkConfiguration interactivelyConfigureNetwork();
    void interactivelyConfigureProcessors();
    void configureRTPProcessor();
    NetworkConfiguration createDefaultNetworkConfiguration();
    std::function<void ()> createStopCallback();
};

#endif	/* OHMCOMM_H */


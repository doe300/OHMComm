#ifndef AUDIOIO_H
#define	AUDIOIO_H

#include "AudioProcessor.h"
#include "Statistics.h"
#include "configuration.h"
#include "ConfigurationMode.h"
#include <vector>
#include <memory> //for std::unique_ptr
#include <iostream>

/*!
 * Base class for Audio framework.
 *
 * Implementations of this class wrap a specific audio library
 */
class AudioHandler
{
public:
    
    //Container for storing info about a single audio-device, library-independent
    struct AudioDevice
    {
        //name of this audio-device
        const std::string name;
        //the maximum number of available output-channels
        //a value of 0 signals a non-output device
        const unsigned int outputChannels;
        //the maximum number of available input-channels
        //a value of 0 signals a non-input device
        const unsigned int inputChannels;
        //whether this device is the default output device
        const bool defaultOutputDevice;
        //whether this device is the default input device
        const bool defaultInputDevice;
        //a bit-mask of natively supported audio-formats
        const unsigned int nativeFormats;
        //a list of all supported sample-rates
        const std::vector<unsigned int> sampleRates;
        
        inline bool isOutputDevice() const
        {
            return outputChannels > 0;
        }
        
        inline bool isInputDevice() const
        {
            return inputChannels > 0;
        }
    };
    
    AudioHandler();
    
    virtual ~AudioHandler();

	/*!
	* Starts processing audio input from the microphone
	*/
    virtual void startRecordingMode() = 0;

	/*!
	* Starts processing audio output to the speakers
	*/
    virtual void startPlaybackMode() = 0;

	/*!
	* Starts processing audio input and output (default  for audio communication)
	*/
    virtual void startDuplexMode() = 0;

	/*!
	* Configures the AudioHandler
	*
	* \param audioConfiguration The Object which contains the configuration
	*/
    virtual void setConfiguration(const AudioConfiguration &audioConfiguration) = 0;

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
	* Returns the actual buffer size, which can be different than in the audio configuration (
	*/
    virtual unsigned int getBufferSize() = 0;

	/*!
	* Prints all included AudioProcessor in the processing order
	*
	* \param outputStream The outputStream for printing (default is std::cout)
	*/
    void printAudioProcessorOrder(std::ostream& outputStream = std::cout) const;

	/*!
	* Adds a AudioProcessor to process chain
	*
	* \param audioProcessor The AudioProcessor to add
	*
	* \return The result of the action
	*/
    auto addProcessor(AudioProcessor *audioProcessor) -> bool;

	/*!
	* Removes a AudioProcessor to process chain
	*
	* \param audioProcessor The AudioProcessor to remove
	*
	* \return The result of the action
	*/
    auto removeAudioProcessor(AudioProcessor *audioProcessor) -> bool;

	/*!
	* Removes a AudioProcessor to process chain
	*
	* \param nameOfAudioProcessor The name of the AudioProcessor to remove
	*
	* \return The result of the action
	*/
    auto removeAudioProcessor(std::string nameOfAudioProcessor) -> bool;

	/*!
	* Clears the process chain (removes all AudioProcessor's)
	*
	* \param nameOfAudioProcessor The name of the AudioProcessor to remove
	*
	* \return The result of the action
	*/
    auto clearAudioProcessors() -> bool;

	/*!
	* Gives a information wheater an AudioProcessor is already added or not
	*
	* \param audioProcessor AudioProcessor object to check
	*
	* \return The result of the action
	*/
    bool hasAudioProcessor(AudioProcessor *audioProcessor) const;

	/*!
	* Gives a information wheater the an AudioProcessor is already added or not
	*
	* \param nameOfAudioProcessor The name of the AudioProcessor to check
	*
	* \return The result of the action
	*/
    bool hasAudioProcessor(std::string nameOfAudioProcessor) const;

	/*!
	* Returns the current AudioConfiguration
	*
	* \return Current AudioConfiguration
	*/
    AudioConfiguration getAudioConfiguration();

	/*!
	* Returns wheater the instance has an AudioConfiguration
	*
	* \return Status of the AudioConfiguration
	*/
    bool isAudioConfigSet() const;

	/*!
	* Returns wheater instance is prepared (ready) for execution
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

protected:
    bool flagAudioConfigSet = false;
    bool flagPrepared = false;


    std::vector<std::unique_ptr<AudioProcessor>> audioProcessors;
    AudioConfiguration audioConfiguration;
    void processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, StreamData *streamData);
    void processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, StreamData *streamData);
    /*!
     * Calls AudioProcessor#configure() for all registered processors
     */
    bool configureAudioProcessors(const std::shared_ptr<ConfigurationMode> configMode);
    bool cleanUpAudioProcessors();
};

#endif

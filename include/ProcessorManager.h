/* 
 * File:   ProcessorManager.h
 * Author: daniel
 *
 * Created on February 22, 2016, 11:17 AM
 */

#ifndef PROCESSORMANAGER_H
#define	PROCESSORMANAGER_H

#include <iostream>
#include <memory>
#include <vector>

#include "configuration.h"
#include "AudioDevice.h"
#include "AudioProcessor.h"
#include "Statistics.h"

/*!
 * Class for storing and managing the active audio-processors
 */
class ProcessorManager
{
public:
    ProcessorManager();

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
    bool addProcessor(AudioProcessor *audioProcessor);

    /*!
     * Removes a AudioProcessor to process chain
     *
     * \param audioProcessor The AudioProcessor to remove
     *
     * \return The result of the action
     */
    bool removeAudioProcessor(AudioProcessor *audioProcessor);

    /*!
     * Removes a AudioProcessor to process chain
     *
     * \param nameOfAudioProcessor The name of the AudioProcessor to remove
     *
     * \return The result of the action
     */
    bool removeAudioProcessor(std::string nameOfAudioProcessor);

    /*!
     * Clears the process chain (removes all AudioProcessor's)
     *
     * \param nameOfAudioProcessor The name of the AudioProcessor to remove
     *
     * \return The result of the action
     */
    bool clearAudioProcessors();

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

    unsigned int processAudioOutput(void *outputBuffer, const unsigned int &outputBufferByteSize, StreamData *streamData);
    unsigned int processAudioInput(void *inputBuffer, const unsigned int &inputBufferByteSize, StreamData *streamData);
    
    /*!
     * Calls AudioProcessor#configure() for all registered processors
     */
    bool configureAudioProcessors(const AudioConfiguration& audioConfiguration, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);
    
    /*!
     * Calls AudioProcessor#startup() for all registered processors
     */
    void startupAudioProcessors();
    
    bool cleanUpAudioProcessors();
    
    /*!
     * "Asks" the AudioProcessors for supported audio-configuration and uses the sample-rate, frame-size and
     * number of samples per package all processors can agree on
     *
     * \return whether all processors could agree on a value for every field
     */
    bool queryProcessorSupport(AudioConfiguration& audioConfiguration, const AudioDevice& inputDevice);
    
private:
    std::vector<std::unique_ptr<AudioProcessor>> audioProcessors;
    
    /*!
     * Returns the best match for the number of buffered frames according to all processors
     */
    unsigned int findOptimalBufferSize(unsigned int defaultBufferSize, unsigned int sampleRate);

    /*!
     * Automatically selects the best audio format out of the supported formats
     */
    static unsigned int autoSelectAudioFormat(unsigned int supportedFormats);

    /*!
     * Maps the supported sample-rates from the device to the flags specified in AudioConfiguration
     */
    static unsigned int mapDeviceSampleRates(std::vector<unsigned int> sampleRates);
};

#endif	/* PROCESSORMANAGER_H */


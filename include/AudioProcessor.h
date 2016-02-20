#ifndef AUDIOPROCESSOR_H
#define	AUDIOPROCESSOR_H

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>   //std::shared_ptr

#include "configuration.h"
#include "ConfigurationMode.h"
#include "rtp/RTPHeader.h"

/*!
 * Information about the stream to be passed to the process-methods of AudioProcessor.
 * Details of the values are specified by the used AudioIO-implementation
 */
struct StreamData
{
    /*!
     * The number of frames in the buffer
     */
    unsigned int nBufferFrames;

    /*!
     * The current time in the stream (i.e. elapsed microseconds)
     */
    unsigned int streamTime;

    /*!
     * The maximum number of bytes to be stored in the input/output-buffer
     */
    unsigned int maxBufferSize;

    /*!
     * Whether the current package is a silent-package.
     * A silence-package has all samples set to a volume of zero
     */
    bool isSilentPackage;
};

/*!
 * Abstract super-type for all classes used for intermediate handling of the input/output stream.
 *
 * An implementation of this class may be used to encode/decode, filter or compress/decompress the input- and output-streams.
 *
 * Processors can be chained, i.e. an output stream can be filtered -> encoded -> compressed.
 * The appertaining input-stream then will be decompressed -> decoded.
 */
class AudioProcessor
{
public:
    /*!
     * This AudioProcessor supports any buffer-length
     */
    static constexpr int BUFFER_SIZE_ANY{1};
    
    AudioProcessor(const std::string name);

    virtual ~AudioProcessor()
    {
        //needs a virtual destructor to be overridden correctly
    }

    /*!
     * \return the unique name of this audio-processor
     */
    const std::string getName() const;

    /*!
     * This method returns the OR'ed flags of the supported audio-formats
     *
     * Available flags are all AUDIO_FORMAT_XXX from AudioConfiguration
     *
     * \return The supported audio-formats
     */
    virtual unsigned int getSupportedAudioFormats() const = 0;

    /*!
     * This method returns the OR'ed flags of the supported sample-rates
     *
     * Available flags are all SAMPLE_RATE_XXX from AudioConfiguration
     *
     * \return The supported sample-rates
     */
    virtual unsigned int getSupportedSampleRates() const = 0;

    /*!
     * This method returns all supported buffer-sizes for the given sample-rate.
     * The buffer-size specified here is the number of samples buffered to be processed together in one processXXX(...) call.
     *
     * Any AudioProcessor which supports an arbitrary buffer-size should include BUFFER_SIZE_ANY at lowest priority.
     *
     * For example a processor supporting all buffer-sizes without any preferences returns {BUFFER_SIZE_ANY}.
     * Any processor preferring specific buffer-sizes returns them in descending priority, e.g. {512, 256, BUFFER_SIZE_ANY}
     * prefers 512 then 256 and if none of these can be matched, any other size.
     *
     * \param sampleRate The chosen sample-rate
     *
     * \return A list of buffer-sizes sorted in descending priority
     */
    virtual const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const = 0;
    
    /*!
     * The payload-type returned by this method has no decision value. 
     * It is only used to set the correct payload-type to the RTP packages.
     * 
     * Additionally, only the payload-type of the last processor in the chain is used.
     * Because this processor determines the actual type of the payload
     * 
     * \return The supported payload-type
     */
    virtual PayloadType getSupportedPlayloadType() const;

    /*!
     * Overwrite this method, if this AudioProcessor needs configuration
     * 
     * NOTE: the audio-configuration in the configuration-mode is not valid, use the audioConfig-parameter!
     *
     * \param audioConfig The valid AudioConfiguration
     * \param configMode The ConfigurationMode to retrieve custom configuration-values from
     *
     */
    virtual bool configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode);

    /*!
     * Counterpart of configure(). This method is called, when the object is not needed any longer.
     */
    virtual bool cleanUp();


    /*!
     * The actual processing methods. processInputData is the counterpart of processInputData
     *
     * \param inputBuffer The buffer to read/write the data from
     *
     * \param inputBufferByteSize The actual number of valid bytes stored in the buffer
     *
     * \param userData A StreamData-object storing additional information about the stream
     *
     * \return the new number of valid bytes in the inputBuffer, maximal StreamData#maxBufferSize
     */
    virtual unsigned int processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData) = 0;

    /*!
     * The actual processing methods. processInputData is the counterpart of processInputData
     *
     * \param outputBuffer The buffer to read/write the data from
     *
     * \param outputBufferByteSize The actual number of valid bytes stored in the buffer
     *
     * \param userData A StreamData-object storing additional information about the stream
     *
     * \return the new number of valid bytes in the outputBuffer, maximal StreamData#maxBufferSize
     */
    virtual unsigned int processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData) = 0;
private:
    const std::string name;
};



#endif

/* 
 * File:   InteractiveConfiguration.cpp
 * Author: daniel
 * 
 * Created on November 30, 2015, 5:33 PM
 */

#include "config/InteractiveConfiguration.h"

#include "AudioHandlerFactory.h"
#include "AudioProcessorFactory.h"

bool InteractiveConfiguration::runConfiguration()
{
    if(isConfigurationDone)
    {
        return true;
    }
    //interactive audio configuration
    if (!UserInput::inputBoolean("Load default audio config?"))
    {
        const std::vector<std::string> audioHandlers = AudioHandlerFactory::getAudioHandlerNames();
        //manually configure audio-handler, so manually select it
        if(audioHandlers.size() > 1)
        {
            //only let user choose if there is more than 1 audio-handler
            audioHandlerName = UserInput::selectOption("Select audio handler", audioHandlers, AudioHandlerFactory::getDefaultAudioHandlerName());
        }
        else
        {
            audioHandlerName = *audioHandlers.begin();
        }
        std::cout << "Using AudioHandler: " << audioHandlerName << std::endl;
        interactivelyConfigureAudioDevices();
        useDefaultAudioConfig = false;
    }
    else
    {
        //use RtAudioWrapper as default implementation
        audioHandlerName = AudioHandlerFactory::getDefaultAudioHandlerName();
        useDefaultAudioConfig = true;
    }
    //interactive network configuration
    if (!UserInput::inputBoolean("Load default network config?"))
    {
        interactivelyConfigureNetwork();
    }
    else
    {
        createDefaultNetworkConfiguration();
    }
    waitForConfigurationRequest = UserInput::inputBoolean("Enable configuration-requests?", false);

    interactivelyConfigureProcessors();

    isConfigurationDone = true;
    return true;
}

const std::string InteractiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const
{
    const unsigned char flags = defaultValue.empty() ? 0 : UserInput::INPUT_USE_DEFAULT;
    return UserInput::inputString(message, defaultValue, flags);
}

int InteractiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const
{
    const unsigned char flags = UserInput::INPUT_ALLOW_NEGATIVE | UserInput::INPUT_ALLOW_ZERO | UserInput::INPUT_USE_DEFAULT;
    return UserInput::inputNumber(message, defaultValue, flags);
}

bool InteractiveConfiguration::getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const
{
    return UserInput::inputBoolean(message, defaultValue, UserInput::INPUT_USE_DEFAULT);
}

bool InteractiveConfiguration::isCustomConfigurationSet(const std::string key, const std::string message) const
{
    return UserInput::inputBoolean(message, false, 0);
}

void InteractiveConfiguration::interactivelyConfigureAudioDevices()
{
    UserInput::printSection("Audio configuration");

    RtAudio AudioDevices;
    // Determine the number of available Audio Devices
    unsigned int availableAudioDevices = AudioDevices.getDeviceCount();
    std::vector<std::string> audioDeviceNames(availableAudioDevices);

    std::cout << "Available Audio Devices: " << std::endl;
    std::cout << std::endl;
    // printing available Audio Devices
    RtAudio::DeviceInfo DeviceInfo;

    unsigned int DefaultOutputDeviceID = 0;
    unsigned int DefaultInputDeviceID = 0;

    for (unsigned int i = 0 ; i < availableAudioDevices ; i++)
    {
        DeviceInfo = AudioDevices.getDeviceInfo(i);
        if (DeviceInfo.probed == true) //Audio Device successfully probed
        {
            std::cout << "Device ID: " << i << std::endl;
            std::cout << "Device Name = " << DeviceInfo.name << std::endl;
            std::cout << "Maximum output channels = " << DeviceInfo.outputChannels << std::endl;
            std::cout << "Maximum input channels = " << DeviceInfo.inputChannels << std::endl;
            std::cout << "Maximum duplex channels = " << DeviceInfo.duplexChannels << std::endl;
            std::cout << "Default output: ";
            if (DeviceInfo.isDefaultOutput == true)
            {
                std::cout << "Yes" << std::endl;
                DefaultOutputDeviceID = i;

            }
            else
            {
                std::cout << "No" << std::endl;
            }
            std::cout << "Default input: ";
            if (DeviceInfo.isDefaultInput == true)
            {
                std::cout << "Yes" << std::endl;
                DefaultInputDeviceID = i;
            }
            else
            {
                std::cout << "No" << std::endl;
            }
            std::cout << "Supported Sample Rates: ";
            for (unsigned int sampleRate : DeviceInfo.sampleRates)
            {
                std::cout << sampleRate << " ";
            }
            std::cout << std::endl;
            std::cout << "Native Audio Formats Flag = " << DeviceInfo.nativeFormats << std::endl;
            std::cout << std::endl;

            audioDeviceNames[i] = (DeviceInfo.name + (DeviceInfo.isDefaultInput && DeviceInfo.isDefaultOutput ? " (default in/out)" :
                (DeviceInfo.isDefaultInput ? " (default in)" : (DeviceInfo.isDefaultOutput ? " (default out)" : ""))));
        }
    }

    //Choose output Device
    unsigned int OutputDeviceID = UserInput::selectOptionIndex("Choose output Device ID", audioDeviceNames, DefaultOutputDeviceID);

    RtAudio::DeviceInfo OutputDeviceInfo = AudioDevices.getDeviceInfo(OutputDeviceID);

    //Configure ID of the Output Audio Device
    audioConfig.outputDeviceID = OutputDeviceID;
    std::cout << "-> Using output Device ID: " << audioConfig.outputDeviceID << std::endl;
    std::cout << "-> Using output Device Name: " << OutputDeviceInfo.name << std::endl;

    //Configure outputDeviceChannels (we always use stereo)
    audioConfig.outputDeviceChannels = 2;

    //sample-rate, audio-format and buffer-size is defined by the enabled Processors so we can skip them

    //Choose input Device
    unsigned int InputDeviceID = UserInput::selectOptionIndex("Choose input Device ID", audioDeviceNames, DefaultInputDeviceID);

    RtAudio::DeviceInfo InputDeviceInfo = AudioDevices.getDeviceInfo(InputDeviceID);

    //Configure ID of the Input Audio Device
    audioConfig.inputDeviceID = InputDeviceID;
    std::cout << "-> Using input Device ID " << audioConfig.inputDeviceID << std::endl;
    std::cout << "-> Using input Device Name: " << InputDeviceInfo.name << std::endl;

    //Configure inputDeviceChannels (we always use stereo)
    audioConfig.inputDeviceChannels = 2;

    //configure whether to force audio-format and sample-rate
    std::vector<std::string> audioFormats = {
        "let audio-handler decide",
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT8, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT16, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT24, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_SINT32, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT32, true),
        AudioConfiguration::getAudioFormatDescription(AudioConfiguration::AUDIO_FORMAT_FLOAT64, true)
    };
    unsigned int selectedAudioFormatIndex = UserInput::selectOptionIndex("Select audio-format", audioFormats, 0);
    if(selectedAudioFormatIndex != 0)
    {
        //must deduct 1 before shifting, because 2^0 = 1 but in the input option 0 is default-value
        audioConfig.forceAudioFormatFlag = 1 << (selectedAudioFormatIndex-1);
    }

    unsigned int selectedSampleRate = UserInput::inputNumber("Input a sample-rate [use 0 for default]", 0, UserInput::INPUT_ALLOW_ZERO|UserInput::INPUT_USE_DEFAULT);
    if(selectedSampleRate > 0)
    {
        audioConfig.forceSampleRate = selectedSampleRate;
    }
}

void InteractiveConfiguration::interactivelyConfigureNetwork()
{
    UserInput::printSection("Network configuration");

    //1. remote address
    std::string ipString = UserInput::inputString("1. Input destination IP address");
    networkConfig.remoteIPAddress = ipString;

    //2. remote and local ports
    int destPort = UserInput::inputNumber("2. Input destination port", 0, 0);
    int localPort = UserInput::inputNumber("3. Input local port", 0, 0);
    networkConfig.remotePort = destPort;
    networkConfig.localPort = localPort;

    std::cout << "Network configuration set." << std::endl;
}

void InteractiveConfiguration::interactivelyConfigureProcessors()
{
    std::vector<std::string> audioProcessors = AudioProcessorFactory::getAudioProcessorNames();
    //interactive processor configuration
    profileProcessors = UserInput::inputBoolean("Create profiler for audio-processors?");
    logToFile = UserInput::inputBoolean("Log statistics and profiling-results to file?");
    if(logToFile)
    {
        logFileName = UserInput::inputString("Type log-file name");
    }
    // add processors to the process chain
    audioProcessors.push_back("End");
    unsigned int selectedIndex;
    std::cout << "The AudioProcessors should be added in the order they are used on the sending side!" << std::endl;
    while((selectedIndex = UserInput::selectOptionIndex("Select next AudioProcessor to add", audioProcessors, audioProcessors.size()-1)) != audioProcessors.size()-1)
    {
        processorNames.push_back(audioProcessors.at(selectedIndex));
        audioProcessors.at(selectedIndex) = audioProcessors.at(selectedIndex) + " (added)";
    }
}
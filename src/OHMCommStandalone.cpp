
#include <memory>
#include "OHMComm.h"
#include "processors/AudioProcessorFactory.h"
#include "audio/AudioHandlerFactory.h"

#include "config/InteractiveConfiguration.h"
#include "config/ParameterConfiguration.h"
#include "sip/SIPConfiguration.h"

using namespace ohmcomm;

int main(int argc, char* argv[])
{
    ////
    // Configuration
    ////

    std::unique_ptr<OHMComm> ohmComm;
    Parameters params(AudioHandlerFactory::getAudioHandlerNames(), AudioProcessorFactory::getAudioProcessorNames());
    if(params.parseParameters(argc, argv))
    {
        if(params.isParameterSet(Parameters::LIST_AUDIO_DEVICES))
        {
            //XXX move to Parameters, but creates dependency on audio-handler
            std::unique_ptr<AudioHandler> handler;
            if(params.isParameterSet(Parameters::AUDIO_HANDLER))
                handler = AudioHandlerFactory::getAudioHandler(params.getParameterValue(Parameters::AUDIO_HANDLER));
            else
                handler = AudioHandlerFactory::getAudioHandler(AudioHandlerFactory::getDefaultAudioHandlerName());           
            std::cout << std::endl;
            std::cout << "Listing local audio-devices:" << std::endl;
            for(const AudioDevice& device : handler->getAudioDevices())
            {
                std::cout << "\tName: " << device.name << std::endl;
                std::cout << "\tInput Channels: " << device.inputChannels << std::endl;
                std::cout << "\tOutput Channels: " << device.outputChannels << std::endl;
                std::cout << "\tDefault input device: " << (device.defaultInputDevice ? "true" : "false") << std::endl;
                std::cout << "\tDefault output device: " << (device.defaultOutputDevice ? "true" : "false") << std::endl;
                std::cout << "\tNative audio formats: " << std::endl;
                for(uint16_t i = 1; i <= AudioConfiguration::AUDIO_FORMAT_FLOAT64; i <<=1)
                {
                    std::cout << "\t\t" << AudioConfiguration::getAudioFormatDescription(i, false) << std::endl;
                }
                std::cout << "\tNative sample rates: ";
                for(auto sampleRate : device.sampleRates)
                {
                    std::cout << sampleRate << ' ';
                }
                std::cout  << std::endl << std::endl;
            }
            exit(0);
        }
        if(params.isParameterSet(Parameters::SIP_LOCAL_PORT) || params.isParameterSet(Parameters::SIP_REMOTE_PORT) || params.isParameterSet(Parameters::SIP_REGISTER_USER))
        {
            NetworkConfiguration sipConfig{0};
            sipConfig.remoteIPAddress = Utility::getAddressForHostName(params.getParameterValue(Parameters::REMOTE_ADDRESS));
            sipConfig.remotePort = atoi(params.getParameterValue(Parameters::SIP_REMOTE_PORT).data());
            sipConfig.localPort = atoi(params.getParameterValue(Parameters::SIP_LOCAL_PORT).data());
            ohmComm.reset(new OHMComm(new sip::SIPConfiguration(params, sipConfig)));
        }
        else
        {
            ohmComm.reset(new OHMComm(new ParameterConfiguration(params)));
        }
    }
    else
    {
        ohmComm.reset(new OHMComm(new InteractiveConfiguration()));
    }

    ////
    // Startup
    ////

    if(!ohmComm->isConfigurationDone(true))
    {
        std::cerr << "Failed to configure OHMComm!" << std::endl;
        return 1;
    }
    ohmComm->startAudioThreads();

    // wait for exit
    std::cout << "Type Enter to exit..." << std::endl;
    Utility::waitForUserInput(-1);

    ohmComm->stopAudioThreads();

    return 0;
}

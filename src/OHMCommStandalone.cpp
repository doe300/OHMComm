#include "OHMComm.h"
#include "AudioProcessorFactory.h"

int main(int argc, char* argv[])
{
    ////
    // Configuration
    ////

    OHMComm* ohmComm;
    Parameters params(AudioHandlerFactory::getAudioHandlerNames(), AudioProcessorFactory::getAudioProcessorNames());
    if(params.parseParameters(argc, argv))
    {
        if(params.isParameterSet(Parameters::PASSIVE_CONFIGURATION))
        {
            NetworkConfiguration networkConfig{0};
            networkConfig.remoteIPAddress = params.getParameterValue(Parameters::REMOTE_ADDRESS);
            networkConfig.remotePort = atoi(params.getParameterValue(Parameters::REMOTE_PORT).data());
            networkConfig.localPort = atoi(params.getParameterValue(Parameters::LOCAL_PORT).data());
            ohmComm = new OHMComm(new PassiveConfiguration(networkConfig));
        }
        else
        {
            ohmComm = new OHMComm(new ParameterConfiguration(params));
        }
    }
    else
    {
        ohmComm = new OHMComm(new InteractiveConfiguration());
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

    char input;
    // wait for exit
    std::cout << "Type Enter to exit" << std::endl;
    std::cin >> input;

    ohmComm->stopAudioThreads();

    delete ohmComm;

    return 0;
}

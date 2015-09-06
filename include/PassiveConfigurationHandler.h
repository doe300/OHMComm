/* 
 * File:   PassiveConfigurationHandler.h
 * Author: daniel
 *
 * Created on August 29, 2015, 6:04 PM
 */

#ifndef PASSIVECONFIGURATIONHANDLER_H
#define	PASSIVECONFIGURATIONHANDLER_H

#include <thread>

#include "ConfigurationMode.h"
#include "NetworkWrapper.h"

/*!
 * Listener for passive configuration-requests
 */
class PassiveConfigurationHandler
{
public:
    PassiveConfigurationHandler(std::shared_ptr<NetworkWrapper> networkWrapper, const std::shared_ptr<ConfigurationMode> configMode);
    
    ~PassiveConfigurationHandler();
    
    void waitForConfigurationRequest();
private:
    std::thread listenerThread;
    const std::shared_ptr<ConfigurationMode> configMode;
    std::shared_ptr<NetworkWrapper> networkWrapper;
    
    void runThread();
};

#endif	/* PASSIVECONFIGURATIONHANDLER_H */


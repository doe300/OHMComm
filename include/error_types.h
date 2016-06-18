/* 
 * File:   configuration_error.h
 * Author: daniel
 *
 * Created on March 11, 2016, 1:41 PM
 */

#ifndef OHMCOMM_ERRORTYPES_H
#define	OHMCOMM_ERRORTYPES_H

#include <stdexcept>

namespace ohmcomm
{
    /*!
     * Error-class for failed configurations (e.g. of audio-processors or -libraries)
     */
    class configuration_error : public std::runtime_error
    {
    public:

        explicit configuration_error(const std::string& component, const std::string& cause) : runtime_error((component + ": ") + cause)
        {
        }
        
        configuration_error(const char* component, const char* cause) : runtime_error((std::string(component) + ": ") + cause)
        {
            
        }

        virtual ~configuration_error()
        {

        }
    };
    
    /*!
     * Error-type for exceptions while starting or running playback
     */
    class playback_error : public std::runtime_error
    {
    public:
        
        explicit playback_error(const std::string& component, const std::string& error)  : runtime_error((component + ": ") + error)
        {
            
        }
        
        playback_error(const char* component, const char* error)  : runtime_error((std::string(component) + ": ") + error)
        {
            
        }
        
        virtual ~playback_error()
        {

        }
    };
}
#endif	/* OHMCOMM_ERRORTYPES_H */


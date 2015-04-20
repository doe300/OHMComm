/* 
 * File:   UDPWrapper.h
 * Author: daniel
 *
 * Created on April 1, 2015, 10:37 AM
 */

#ifndef UDPWRAPPER_H
#define	UDPWRAPPER_H

#include "NetworkWrapper.h"


class UDPWrapper: public NetworkWrapper
{
public:
    UDPWrapper();
    UDPWrapper(const UDPWrapper& orig);
    virtual ~UDPWrapper();
    
    int process(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData);
    
    void configure();
};

#endif	/* UDPWRAPPER_H */


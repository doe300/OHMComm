/* 
 * File:   RTCPData.h
 * Author: daniel
 *
 * Created on February 25, 2016, 10:00 AM
 */

#ifndef RTCPDATA_H
#define	RTCPDATA_H

#include <chrono>
#include <vector>

#include "RTCPHeader.h"

//Statistical and informational RTCP data
struct RTCPData
{
    //the timestamp of the reception of the last RTCP SR package sent by this participant.
    //for the local participant, this is the timestamp of the last SR sent
    std::chrono::steady_clock::time_point lastSRTimestamp;
    //the list of source descriptions for this participant
    //can be used to display user-friendly names and descriptions for the participant
    std::vector<SourceDescription> sourceDescriptions;
    
    RTCPData() : lastSRTimestamp(std::chrono::steady_clock::duration::zero())
    {
        
    }
    
    
};

#endif	/* RTCPDATA_H */


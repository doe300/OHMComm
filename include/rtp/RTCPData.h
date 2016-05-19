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

namespace ohmcomm
{
    namespace rtp
    {
        /*!
         *\brief Statistical and informational RTCP data
         * 
         * Contains the list of source descriptions for this participant, which
         * can be used to display user-friendly names and descriptions for the participant
         */
        struct RTCPData : public KeyValuePairs<SourceDescription>
        {
            //the timestamp of the reception of the last RTCP SR package sent by this participant.
            //for the local participant, this is the timestamp of the last SR sent
            std::chrono::steady_clock::time_point lastSRTimestamp;

            RTCPData() : lastSRTimestamp(std::chrono::steady_clock::duration::zero())
            {

            }


        };
    }
}
#endif	/* RTCPDATA_H */


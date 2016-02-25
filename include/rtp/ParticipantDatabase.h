/* 
 * File:   RTPData.h
 * Author: daniel
 *
 * Created on November 20, 2015, 10:46 AM
 */

#ifndef PARTICIPANT_DATABASE_H
#define	PARTICIPANT_DATABASE_H

#include <map>
#include <chrono>
#include <memory>

#include "RTCPData.h"

//Forward declaration for pointer to SIP user-agent data
struct SIPUserAgent;

//Forward declaration for pointer to statistical and informational RTCP data
struct RTCPData;

/*!
 * Global data store for a single participant in an RTP session
 */
struct Participant
{
    const bool isLocalParticipant;
    //the SSRC of the participant
    //XXX make const
    uint32_t ssrc;
    //the payload-type sent and received by this participant
    char payloadType;
    //the RTP timestamp of the first package sent by this participant
    uint32_t initialRTPTimestamp;
    //the currently highest sequence number sent by this participant
    uint32_t extendedHighestSequenceNumber;
    //the current estimated jitter between RTP packages sent by this participant, undefined for local participant
    double interarrivalJitter;
    //the timestamp of the reception of the last package (RTP/RTCP) sent by this participant, undefined for local user
    std::chrono::steady_clock::time_point lastPackageReceived;
    //the amount of packages lost originating from this participant
    uint32_t packagesLost;
    //the amount of packages received originating from this participant
    //equals the amount of packages sent for the local participant
    uint32_t totalPackages;
    //the total amount of octets/bytes sent/received for this participant
    uint32_t totalBytes;
    //the SIP user-agent data
    //we can't use unique_ptr here, because SIPUserAgent is incomplete
    std::shared_ptr<SIPUserAgent> userAgent;
    //the RTCP participant-data
    std::shared_ptr<RTCPData> rtcpData;
    
    Participant(const uint32_t ssrc, const bool localParticipant) : isLocalParticipant(localParticipant), ssrc(ssrc), 
        initialRTPTimestamp(-1), extendedHighestSequenceNumber(0), lastPackageReceived(std::chrono::steady_clock::time_point::min()), 
        packagesLost(0), totalPackages(0), totalBytes(0), userAgent(nullptr), rtcpData(nullptr)
    {
        
    }
    
    /*!
     * \return the fraction of packages lost in 1/256
     */
    inline uint8_t getFractionLost() const
    {
        return (long)(((double)packagesLost/(double)totalPackages) * 256);
    }
};

class ParticipantDatabase
{
public:
    
    /*!
     * \return the participant for this instance
     */
    static Participant& self()
    {
        if(localSSRC < 0)
            initLocalParticipant();
        return participants.at(localSSRC);
    }
    
    /*!
     * \param ssrc the SSRC of the remote participant
     * 
     * \return the n-th remote participant
     */
    static Participant& remote(const uint32_t ssrc = 0)
    {
        if(!isInDatabase(ssrc))
            participants.emplace(std::make_pair(ssrc, Participant{ssrc, false}));
        return participants.at(ssrc);
    }
    
    /*!
     * \param ssrc the SSRC to check
     * 
     * \return whether a participant with this SSRC exists in the local database
     */
    static bool isInDatabase(const uint32_t ssrc)
    {
        return participants.find(ssrc) != participants.end();
    }
    
    /*!
     * \return a read-only map of all currently registered participants
     */
    static const std::map<uint32_t, Participant> getAllParticipants()
    {
        return participants;
    }
    
    static bool removeParticipant(const uint32_t ssrc)
    {
        if(ssrc == localSSRC)
            //prevent local from being removed
            return false;
    }
    
private:
    static int64_t localSSRC;
    static std::map<uint32_t, Participant> participants;
    
    static void initLocalParticipant();
};

#endif	/* PARTICIPANT_DATABASE_H */


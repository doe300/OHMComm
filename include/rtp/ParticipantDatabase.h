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

//Forward declaration for pointer to SIP user-agent data
class SIPUserAgent;

/*!
 * Global data store for a single participant in an RTP session
 */
struct Participant
{
    const bool isLocalParticipant;
    //the SSRC of the participant
    //XXX make const
    uint32_t ssrc;
    //the RTP timestamp of the first package sent by this participant
    uint32_t initialRTPTimestamp;
    //the currently highest sequence number sent by this participant
    uint32_t extendedHighestSequenceNumber;
    //the current estimated jitter between packages sent by this participant, undefined for local participant
    double interarrivalJitter;
    //the timestamp of the reception of the last package (RTP/RTCP) sent by this participant, undefined for local user
    std::chrono::steady_clock::time_point lastPackageReceived;
    //the timestamp of the reception of the last RTCP SR package sent by this participant.
    //for the local participant, this is the timestamp of the last SR sent
    std::chrono::steady_clock::time_point lastSRTimestamp;
    //the SIP user-agent data
    //we can't use unique_ptr here, because SIPUserAgent is incomplete
    std::shared_ptr<SIPUserAgent> userAgent;
    
    Participant(const uint32_t ssrc, const bool localParticipant) : isLocalParticipant(localParticipant), ssrc(ssrc), initialRTPTimestamp(-1), extendedHighestSequenceNumber(0),
        lastPackageReceived(std::chrono::steady_clock::time_point::min()),
        lastSRTimestamp(std::chrono::steady_clock::time_point::min()), userAgent(nullptr)
    {
        
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


/* 
 * File:   RTPData.h
 * Author: daniel
 *
 * Created on November 20, 2015, 10:46 AM
 */

#ifndef PARTICIPANT_DATABASE_H
#define	PARTICIPANT_DATABASE_H

#include <string>
#include <chrono>

/*!
 * Information about a participant of a session relevant for the SIP protocol
 */
struct SIPUserAgent
{
    std::string userName;
    std::string hostName;
    std::string tag;
    std::string ipAddress;
    unsigned short port;

    /*!
     * \return the SIP-URI in the format sip:<userName>@<hostName|ipAddress>[:<port>]
     */
    const inline std::string getSIPURI() const
    {
        //sip:user@host[:port]
        return (std::string("sip:") + userName + "@") + (hostName.empty() ? ipAddress : hostName) + (port > 0 ? std::string(":") + std::to_string(port): std::string());
    }
};

struct Participant
{
    //the SSRC of the participant
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
    SIPUserAgent userAgent;
};

class ParticipantDatabase
{
public:
    
    static constexpr unsigned int MAX_PARTICIPANTS{16};
    
    /*!
     * \return the participant for this instance
     */
    static Participant& self()
    {
        return participants[PARTICIPANT_SELF];
    }
    
    /*!
     * \param remoteIndex the index of the remote participant
     * 
     * \return the n-th remote participant
     */
    static Participant& remote(const unsigned int remoteIndex = 0)
    {
        return participants[PARTICIPANT_REMOTE + remoteIndex];
    }
    
private:
    static constexpr unsigned int PARTICIPANT_SELF{0};
    static constexpr unsigned int PARTICIPANT_REMOTE{1};
    /* Declared in ProcessorRTP.cpp */
    static Participant participants[MAX_PARTICIPANTS];
};

#endif	/* PARTICIPANT_DATABASE_H */


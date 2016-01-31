/* 
 * File:   RTPData.h
 * Author: daniel
 *
 * Created on November 20, 2015, 10:46 AM
 */

#ifndef PARTICIPANT_DATABASE_H
#define	PARTICIPANT_DATABASE_H

#include <string>

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

    /* Declared in SIPHandler.cpp */
    const std::string getSIPURI() const;
};

struct Participant
{
    uint32_t ssrc;
    uint32_t initialRTPTimestamp;
    uint32_t extendedHighestSequenceNumber;
    double interarrivalJitter;
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


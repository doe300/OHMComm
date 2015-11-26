/* 
 * File:   RTPData.h
 * Author: daniel
 *
 * Created on November 20, 2015, 10:46 AM
 */

#ifndef PARTICIPANT_DATABASE_H
#define	PARTICIPANT_DATABASE_H

#include <string>

struct Participant
{
    uint32_t ssrc;
    uint32_t initialRTPTimestamp;
    uint32_t extendedHighestSequenceNumber;
    double interarrivalJitter;
};

static const unsigned int PARTICIPANT_SELF = 0;
static const unsigned int PARTICIPANT_REMOTE = 1;

/* Declared in ProcessorRTP.cpp */
extern Participant participantDatabase[2];

#endif	/* PARTICIPANT_DATABASE_H */


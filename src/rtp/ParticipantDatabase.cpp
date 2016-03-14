
#include <random>
#include <stdlib.h> //abs for clang
#include <iostream>

#include "rtp/ParticipantDatabase.h"

using namespace ohmcomm::rtp;

float Participant::calculateInterarrivalJitter(uint32_t sentTimestamp, uint32_t receptionTimestamp)
{
    //as of RFC 3550 (A.8):
    //D(i, j)=(Rj - Sj) - (Ri - Si)
    //with (Ri - Si) = lastDelay
    int32_t currentDelay = receptionTimestamp - sentTimestamp;
    int32_t currentDifference = currentDelay - lastDelay;
    lastDelay = currentDelay;

    //Ji = Ji-1 + (|D(i-1, 1)| - Ji-1)/16
    double lastJitter = interarrivalJitter;
    lastJitter = lastJitter + ((float) abs(currentDifference) - lastJitter) / 16.0;
    interarrivalJitter = lastJitter;
    return lastJitter;
}

Participant ParticipantDatabase::localParticipant = initLocalParticipant();
std::map<uint32_t, Participant> ParticipantDatabase::participants{};

Participant ParticipantDatabase::initLocalParticipant()
{
    unsigned int seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 tmp(seed1);
    uint32_t ssrc = tmp();
    return Participant{ssrc, true};
}

void ParticipantDatabase::fireNewRemote(const uint32_t ssrc)
{
    std::cout << "RTP: New remote joined conversation: " << ssrc << std::endl;
}

void ParticipantDatabase::fireRemoveRemote(const uint32_t ssrc)
{
    std::cout << "RTP: Remote left conversation: " << ssrc << std::endl;
}

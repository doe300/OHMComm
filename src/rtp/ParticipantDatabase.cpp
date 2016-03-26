
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
std::vector<std::reference_wrapper<ParticipantListener>> ParticipantDatabase::listeners{};

Participant ParticipantDatabase::initLocalParticipant()
{
    unsigned int seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 tmp(seed1);
    uint32_t ssrc = tmp();
    return Participant{ssrc, true};
}

bool ParticipantDatabase::removeParticipant(const uint32_t ssrc)
{
    if (participants.erase(ssrc) > 0) {
        fireRemoveRemote(ssrc);
        return true;
    }
    return false;
}

void ParticipantDatabase::registerListener(ParticipantListener& listener)
{
    listeners.push_back(std::ref(listener));
}

void ParticipantDatabase::unregisterListener(ParticipantListener& listener)
{
    for (auto it = listeners.begin(); it < listeners.end(); ++it) {
        //compare for same object, so they have same address
        if (&((*it).get()) == &listener) {
            listeners.erase(it);
            return;
        }
    }
}


void ParticipantDatabase::fireNewRemote(const uint32_t ssrc)
{
    std::cout << "RTP: New remote joined conversation: " << ssrc << std::endl;
    for(ParticipantListener& l : listeners)
    {
        l.onRemoteAdded(ssrc);
    }
}

void ParticipantDatabase::fireRemoveRemote(const uint32_t ssrc)
{
    std::cout << "RTP: Remote left conversation: " << ssrc << std::endl;
    for(ParticipantListener& l : listeners)
    {
        l.onRemoteRemoved(ssrc);
    }
}
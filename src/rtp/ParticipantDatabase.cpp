
#include <random>

#include "rtp/ParticipantDatabase.h"

//initial set local SSRC to -1
int64_t ParticipantDatabase::localSSRC{-1};
std::map<uint32_t, Participant> ParticipantDatabase::participants{};

void ParticipantDatabase::initLocalParticipant()
{
    unsigned int seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 tmp(seed1);
    localSSRC = tmp();
    participants.insert(std::make_pair(localSSRC, Participant{(uint32_t)localSSRC, true}));
}

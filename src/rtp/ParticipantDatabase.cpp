
#include <random>

#include "rtp/ParticipantDatabase.h"

using namespace ohmcomm::rtp;

Participant ParticipantDatabase::localParticipant = initLocalParticipant();
std::map<uint32_t, Participant> ParticipantDatabase::participants{};

Participant ParticipantDatabase::initLocalParticipant()
{
    unsigned int seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 tmp(seed1);
    uint32_t ssrc = tmp();
    return Participant{ssrc, true};
}

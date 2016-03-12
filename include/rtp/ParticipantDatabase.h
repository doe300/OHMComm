/* 
 * File:   RTPData.h
 * Author: daniel
 *
 * Created on November 20, 2015, 10:46 AM
 */

#ifndef PARTICIPANT_DATABASE_H
#define	PARTICIPANT_DATABASE_H

#include <stdlib.h> //abs for clang
#include <map>
#include <chrono>
#include <memory>

namespace ohmcomm
{
    namespace rtp
    {

        //Forward declaration for pointer to statistical and informational RTCP data
        struct RTCPData;

        /*!
         * Global data store for a single participant in an RTP session
         */
        struct Participant
        {
        private:
            //the last delay-value
            uint32_t lastDelay;
        public:
            const bool isLocalParticipant;
            //the SSRC of the participant
            const uint32_t ssrc;
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
            //the RTCP participant-data
            //we can't use reference/unique_ptr here, because RTCPData is incomplete at this point
            std::shared_ptr<RTCPData> rtcpData;

            Participant(const uint32_t ssrc, const bool localParticipant) : lastDelay(0), isLocalParticipant(localParticipant), ssrc(ssrc), payloadType(-1),
            initialRTPTimestamp(0), extendedHighestSequenceNumber(0), interarrivalJitter(0), lastPackageReceived(std::chrono::steady_clock::time_point::min()),
            packagesLost(0), totalPackages(0), totalBytes(0), rtcpData(nullptr)
            {

            }

            /*!
             * \return the fraction of packages lost in 1/256
             */
            inline uint8_t getFractionLost() const
            {
                return (long) (((double) packagesLost / (double) totalPackages) * 256);
            }

            /*!
             * Calculates and sets the new interarrival-jitter value
             * 
             * \param sentTimestamp The RTP timestamp (in remote clock) of the last package sent by this participant
             * 
             * \param receptionTimstamp The RTP timestamp (in local clock) of the reception of the last package sent by this participant
             * 
             * \return the newly calculated interarrival-jitter
             */
            float calculateInterarrivalJitter(uint32_t sentTimestamp, uint32_t receptionTimestamp)
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
        };

        class ParticipantDatabase
        {
        public:

            /*!
             * \return the participant for this instance
             */
            static Participant& self()
            {
                return localParticipant;
            }

            /*!
             * \param ssrc the SSRC of the remote participant
             * 
             * \return the n-th remote participant
             */
            static Participant& remote(const uint32_t ssrc)
            {
                if (!isInDatabase(ssrc))
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
             * \return a read-only map of all currently registered remote participants
             */
            static const std::map<uint32_t, Participant>& getAllRemoteParticipants()
            {
                return participants;
            }

            /*!
             * Removes the remote participant with the given SSRC from the list of active participants
             * 
             * \param ssrc The SSRC to remove
             * 
             * \return whether a participant for this SSRC was found and removed
             */
            static bool removeParticipant(const uint32_t ssrc)
            {
                return participants.erase(ssrc) > 0;
            }

        private:
            static Participant localParticipant;
            static std::map<uint32_t, Participant> participants;

            static Participant initLocalParticipant();
        };
    }
}
#endif	/* PARTICIPANT_DATABASE_H */


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
            float calculateInterarrivalJitter(uint32_t sentTimestamp, uint32_t receptionTimestamp);
        };

        class ParticipantDatabase
        {
        public:
            
            /*!
             * \return the participant for this instance
             */
            inline static Participant& self()
            {
                return localParticipant;
            }

            /*!
             * \param ssrc the SSRC of the remote participant
             * 
             * \return the n-th remote participant
             */
            inline static Participant& remote(const uint32_t ssrc)
            {
                if (!isInDatabase(ssrc))
                {
                    participants.emplace(std::make_pair(ssrc, Participant{ssrc, false}));
                    fireNewRemote(ssrc);
                }
                return participants.at(ssrc);
            }

            /*!
             * \param ssrc the SSRC to check
             * 
             * \return whether a participant with this SSRC exists in the local database
             */
            inline static bool isInDatabase(const uint32_t ssrc)
            {
                return participants.find(ssrc) != participants.end();
            }

            /*!
             * \return a read-only map of all currently registered remote participants
             */
            inline static const std::map<uint32_t, Participant>& getAllRemoteParticipants()
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
            inline static bool removeParticipant(const uint32_t ssrc)
            {
                if(participants.erase(ssrc) > 0)
                {
                    fireRemoveRemote(ssrc);
                    return true;
                }
                return false;
            }
            
            /*!
             * \return the number of currently active remote participants
             */
            inline static unsigned int getNumberOfRemotes()
            {
                return participants.size();
            }
            
        private:
            static Participant localParticipant;
            static std::map<uint32_t, Participant> participants;

            static Participant initLocalParticipant();
            
            //fires listeners for new remotes
            static void fireNewRemote(const uint32_t ssrc);
            
            //fires listeners for removal of remotes
            static void fireRemoveRemote(const uint32_t ssrc);
        };
    }
}
#endif	/* PARTICIPANT_DATABASE_H */


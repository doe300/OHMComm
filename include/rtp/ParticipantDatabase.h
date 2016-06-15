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
#include <vector>
#include <functional>
#include <mutex>
#include <string>

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
            //the timestamp if the reception of the first package (RTP/RTCP) from this participant in the current session
            const std::chrono::steady_clock::time_point firstPackageReceived;
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
            firstPackageReceived(std::chrono::steady_clock::now()), packagesLost(0), totalPackages(0), totalBytes(0), rtcpData(nullptr)
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
            
            /*!
             * Sets the remote-address and notifies the listeners
             * 
             * \param address The IP-address of the remote device
             * 
             * \param port The remote-port
             */
            void setRemoteAddress(const std::string& address, const uint16_t port);
        };
        
        /*!
         * Subclasses of this listener will be notified when a new participant is added or an existing one removed
         */
        class ParticipantListener
        {
        public:
            virtual ~ParticipantListener()
            {
                //for correct call to overriding destructor
            }

            /*!
             * This method is triggered when a new remote is added to the list of participants
             */
            virtual void onRemoteAdded(const unsigned int ssrc)
            {
            }
            
            /*!
             * This method is triggered when a new remote has connected (send a message for the first time)
             */
            virtual void onRemoteConnected(const unsigned int ssrc, const std::string& address, const unsigned short port)
            {
            }
            
            /*!
             * This message is triggered when a remote is being removed from the list of participants
             */
            virtual void onRemoteRemoved(const unsigned int ssrc)
            {
            }
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
                std::lock_guard<std::mutex> guard(databaseMutex);
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
            static bool removeParticipant(const uint32_t ssrc);
            
            /*!
             * \return the number of currently active remote participants
             */
            inline static unsigned int getNumberOfRemotes()
            {
                return participants.size();
            }
            
            /*!
             * Registers a new listener for participants
             */
            static void registerListener(ParticipantListener& listener);
            
            /*!
             * Removes an existing listener for participants
             */
            static void unregisterListener(ParticipantListener& listener);
            
        private:
            static std::mutex databaseMutex;
            static Participant localParticipant;
            static std::map<uint32_t, Participant> participants;
            //need to wrap references, since they cannot be stored in vectors
            static std::vector<std::reference_wrapper<ParticipantListener>> listeners;

            static Participant initLocalParticipant();
            
            //fires listeners for new remotes
            static void fireNewRemote(const uint32_t ssrc);
            
            //fires listeners for removal of remotes
            static void fireRemoveRemote(const uint32_t ssrc);
            
            //fires listeners for setting address for remote
            static void fireConnectedRemote(const uint32_t ssrc, const std::string& address, const uint16_t port);
            
            friend struct Participant;
        };
    }
}
#endif	/* PARTICIPANT_DATABASE_H */


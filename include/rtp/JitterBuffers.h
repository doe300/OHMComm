/* 
 * File:   JitterBuffers.h
 * Author: daniel
 *
 * Created on February 25, 2016, 2:00 PM
 */

#ifndef JITTERBUFFERS_H
#define	JITTERBUFFERS_H

#include <map>
#include <mutex>
#include <memory>

#include "RTPBufferHandler.h"

namespace ohmcomm
{
    namespace rtp
    {
        /*!
         * Container for storing and managing all RTP jitter-buffers in use
         */
        class JitterBuffers
        {
        public:
            /*!
             * \param maxCapacity The maximum number of packages to buffer in a single jitter-buffer
             * \param maxDelay The maximum delay in milliseconds before dropping packages
             * \param minBufferPackages The minimum of packages to buffer before returning valid audio-data
             */
            JitterBuffers(const uint16_t maxCapacity, const uint16_t maxDelay, const uint16_t minBufferPackage = 1);

            /*!
             * \param ssrc The SSRC to retrieve (and create) the buffer for
             * 
             * \return the jitter-buffer for the given SSRC
             */
            const std::unique_ptr<RTPBufferHandler>& getBuffer(const uint32_t ssrc);

            /*!
             * Destroys the RTP-buffer for the given SSRC freeing its resources
             */
            void removeBuffer(const uint32_t ssrc);

            /*!
             * Deletes all buffers
             */
            void cleanup();
        private:
            const uint16_t maximumCapacity;
            const uint16_t maximumDelay;
            const uint16_t minBufferPackages;
            std::mutex mutex;
            std::map<uint32_t, std::unique_ptr<RTPBufferHandler>> buffers;
        };
    }
}

#endif	/* JITTERBUFFERS_H */


/* 
 * File:   MulticastNetworkWrapper.h
 * Author: daniel
 *
 * Created on February 27, 2016, 11:47 AM
 */

#ifndef MULTICASTNETWORKWRAPPER_H
#define	MULTICASTNETWORKWRAPPER_H

#include <string>
#include <vector>
#include <utility>

#include "configuration.h"
#include "UDPWrapper.h"

namespace ohmcomm
{

    namespace network
    {

        /*!
         * UDP-based network-wrapper sending packages to multiple destinations
         */
        class MulticastNetworkWrapper : public UDPWrapper
        {
        public:
            MulticastNetworkWrapper(const NetworkConfiguration& initialDestination);

            virtual ~MulticastNetworkWrapper()
            {

            }

            int sendData(const void* buffer, const unsigned int bufferSize) override;

            /*!
             * Adds a new remote address as destination sending packages
             * 
             * \param destinationAddress The IP-address of the remote device
             * 
             * \param destinationPort The remote-port to send to
             * 
             * \return whether the new destination was added
             */
            bool addDestination(const std::string& destinationAddress, const unsigned short destinationPort = DEFAULT_NETWORK_PORT);

            /*!
             * Removes the remote address from the list of destinations
             * 
             * \param destinationAddress The IP-address of the remote device
             * 
             * \param destinationPort The remote port
             * 
             * \return whether the destination was removed
             */
            bool removeDestination(const std::string& destinationAddress, const unsigned short destinationPort = DEFAULT_NETWORK_PORT);

        private:
            //a list of destination-addresses
            std::vector<SocketAddress> destinations;
        };
    }
}
#endif	/* MULTICASTNETWORKWRAPPER_H */


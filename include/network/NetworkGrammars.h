/* 
 * File:   NetworkGrammars.h
 * Author: daniel
 *
 * Created on February 13, 2016, 10:30 AM
 */

#ifndef NETWORKGRAMMARS_H
#define	NETWORKGRAMMARS_H

#include <string>
#include <tuple>

namespace ohmcomm
{

    /*!
     * Utility class to check grammars for common network-related values
     */
    class NetworkGrammars
    {
    public:

        static const int INVALID_PORT;

        /*!
         * Converts the given string into a host-part and a numerical port.
         * An illegal string will be returned as an empty host. If the port is not set, the defaultPort is returned.
         * If the port is illegal, INVALID_PORT will be returned,
         * 
         * \param hostAndPort The string to convert
         * 
         * \param defaultPort The default value for the port, if there is none
         * 
         * \return a tuple consisting of host and port
         */
        static std::tuple<std::string, int> toHostAndPort(const std::string& hostAndPort, const int defaultPort = INVALID_PORT);

        /*!
         * Checks, whether the string is a valid domain name, IPv4 or IPv6 address
         * 
         * \param host The host to check
         * 
         * \return whether the given string is a valid host name of any kind
         */
        static inline bool isValidHost(const std::string& host)
        {
            return isValidDomainName(host) || isIPv4Address(host) || isIPv6Address(host);
        }

        /*!
         * \return whether the string is a valid domain name
         */
        static bool isValidDomainName(const std::string& hostName);

        /*!
         * \return whether the string is a valid IPv4 address
         */
        static bool isIPv4Address(const std::string& address);

        /*!
         * \return whether the string is a valid IPv6 address
         */
        static bool isIPv6Address(const std::string& address);

        /*!
         * \return the passed port-number as integer or INVALID_PORT
         */
        static int toPort(const std::string& port);
    };
}
#endif	/* NETWORKGRAMMARS_H */


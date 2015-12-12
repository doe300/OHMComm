/* 
 * File:   Utility.h
 * Author: daniel
 *
 * Created on December 5, 2015, 4:14 PM
 */

#ifndef UTILITY_H
#define	UTILITY_H

#include <string>
#include <algorithm>
#include <stdexcept>
#include <locale>
/*!
 * Class providing utility methods
 */
class Utility
{
public:
    
    enum class AddressType
    {
        /*!
         * Retrieves the current loopback-address
         */
        ADDRESS_LOOPBACK,
        /*!
         * Retrieves the IP-address of this device inside the local network
         */
        ADDRESS_LOCAL_NETWORK,
        /*!
         * Retrieves the IP-address of this device on the internet. This may be the external IP address of a local NAT or router
         */
        ADDRESS_INTERNET
    };
    
    /*!
     * \return the domain- or host-name of this device, or "unknown" if no such name could be determined
     */
    static std::string getDomainName();

    /*!
     * \return the name of the user logged in, or "unknown" if no such name could be determined
     */
    static std::string getUserName();
    
    /*!
     * NOTE: on devices with multiple network interfaces, this method may return the wrong address
     * 
     * \param addressType The type of the network to retrieve the local address for
     * 
     * \return the external local IP address or an empty string on error
     */
    static std::string getLocalIPAddress(const AddressType addressType = AddressType::ADDRESS_LOCAL_NETWORK );
    
    /*!
     * \param remoteAddress The remote IP-address to get the network-type for
     * 
     * \return the network-type for the network between this device and the device with the given remote-address
     */
    static AddressType getNetworkType(const std::string& remoteAddress);
    
    /*!
     * \param hostName the DNS name to retrieve the IP address for
     * 
     * \return the IPv4 or IPv6 address in the standard textual representation, or an empty string on error
     */
    static std::string getAddressForHostName(const std::string& hostName);

    /*!
     * \param in The string to trim
     * 
     * \return The input string with all leading and trailing white-spaces removed
     */
    static std::string trim(const std::string& in);
    
    /*!
     * \param s1 The first string
     * \param s2 The seconds string
     * 
     * \return whether the strings are equals (ignoring case)
     */
    static bool equalsIgnoreCase(const std::string& s1, const std::string s2);
    
private:
    
    static std::string getExternalLocalIPAddress();
    
    static std::string getExternalNetworkIPAddress();
    
    static bool isLocalNetwork(const std::string& ipAddress);
};

#endif	/* UTILITY_H */


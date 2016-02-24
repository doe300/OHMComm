/* 
 * File:   Utility.h
 * Author: daniel
 *
 * Created on December 5, 2015, 4:14 PM
 */

#ifndef UTILITY_H
#define	UTILITY_H

#include <utility>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>

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
     * If an IP-address is passed to this method, the same address is returned
     * 
     * \param hostName the DNS name to retrieve the IP address for
     * 
     * \return the IPv4 or IPv6 address in the standard textual representation, or an empty string on error
     */
    static std::string getAddressForHostName(const std::string& hostName);
    
    /*!
     * Extracts the IP-address and the port from the given buffer containing a sockaddr_in or sockaddr_in6 object
     * 
     * \param socketAddress the buffer containing the sockaddr_in or sockaddr_in6 object
     * 
     * \param addressLength the length of the buffer
     * 
     * \param isIPv6 whether the given address is an IPv6 address
     * 
     * \return a pair of IP-address and port
     */
    static std::pair<std::string, unsigned short> getSocketAddress(const void* socketAddress, const unsigned int addressLength, const bool isIPv6);

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
    
    /*!
     * \param str The string to replace occurrences in (this will be modified)
     * \param from The occurrence to replace
     * \param to The string to replace with
     * 
     * \return the original string with all occurrences replaced
     */
    static std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    
    /*!
     * Joins the strings in the given vector by concatenating them in the order of the vector.
     * Between consecutive strings, the delimiter is inserted
     * 
     * \param vector The strings to be concatenated
     * \param delimiter The string to insert between the single strings
     * 
     * \return the concatenated string
     */
    static std::string joinStrings(const std::vector<std::string>& vector, const std::string& delimiter = " ");
    
    /*!
     * Decodes an URI-encoded string by reversing the '%'-escaping
     * 
     * \param uri The (possibly) URI-encoded string
     * 
     * \return The decoded string
     */
    static std::string decodeURI(const std::string& uri);
    
    /*!
     * Converts the given number to string using the hexadecimal representation
     * 
     * \param number The number to convert
     * 
     * \return a string-representation of the given number using hex digits
     */
    static std::string toHexString(unsigned int number);
    
    /*!
     * Waits the given time (in milliseconds) for user input and returns the input or -1
     * 
     * \param waitInMS The time in milliseconds to wait for user input. If set to -1, the method will wait indefinitely
     * 
     * \return The key-code of the first character input or -1 if no input was performed
     */
    static int waitForUserInput(const int waitInMS);
    
    /*!
     * Splits a string by the given delimiter, including empty tokens
     * 
     * \param input The string to split
     * 
     * \param delimiter The delimiter to split at
     * 
     * \return a vector of tokens
     */
    static std::vector<std::string> splitString(const std::string& input, const char delimiter);

private:
    
    static std::string getExternalLocalIPAddress();
    
    static std::string getExternalNetworkIPAddress();
    
    static bool isLocalNetwork(const std::string& ipAddress);
};

#endif	/* UTILITY_H */


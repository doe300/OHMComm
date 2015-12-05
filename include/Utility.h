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

/*!
 * Class providing utility methods
 */
class Utility
{
public:
    /*!
     * \return the Domain- or host-name of this device, or "unknown" if no such name could be determined
     */
    static std::string getDomainName();

    /*!
     * \return the name of the user logged in, or "unknown" if no such name could be determined
     */
    static std::string getUserName();

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
    static bool compareIgnoreCase(const std::string& s1, const std::string s2);
};

#endif	/* UTILITY_H */


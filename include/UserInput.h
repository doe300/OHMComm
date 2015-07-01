/* 
 * File:   UserInput.h
 * Author: daniel
 *
 * Created on April 28, 2015, 5:03 PM
 */

#ifndef USERINPUT_H
#define	USERINPUT_H

#include <string>
#include <vector>
#include <iostream>
#include <limits.h>

/*!
 * Utility-class to provide common methods for user-input
 */
class UserInput
{
public:
    
    /*!
     * Prints a section-title in a common formatted way
     * 
     * \param title The title to print
     */
    static void printSection(std::string title);
    /*!
     * Allows the user to input a bool-value.
     * Accepted values are 'Y', 'Yes' and 'N', 'No' (lower and mixed case)
     * 
     * \param message The message to display
     * 
     * Returns the input bool-value
     */
    static bool inputBoolean(std::string message);

    /*!
     * Allows the user to input a line of text
     * 
     * \param message The message to display
     * 
     * Returns the input line as string
     */
    static std::string inputString(std::string message);

    /*!
     * Allows the user to select an option from the given list. The option is selected by typing the option-text
     * 
     * \param message The message to display
     * 
     * \param options The options to choose one from
     * 
     * \param defaultOption The value to return if none of the other options were selected
     * 
     * Returns the option selected
     */
    static std::string selectOption(std::string message, std::vector<std::string> options, std::string defaultOption);

    /*!
     * Allows the user to select an option from the given list. The option is selected by typing its index
     * 
     * \param message The message to display
     * 
     * \param options The options to choose from
     * 
     * \param defaultIndex The default index to select if no existing index was choosen
     * 
     * Returns the selected index
     */
    static unsigned int selectOptionIndex(std::string message, std::vector<std::string> options, unsigned int defaultIndex);

    /*!
     * Allows the user to input a integer number.
     * 
     * \param message The message to display
     * 
     * \param allowZero Whether the value zero '0' is allowed
     * 
     * \param allowNegative Whether negative numbers are allowed
     * 
     * Returns the number typed
     */
    static int inputNumber(std::string message, bool allowZero, bool allowNegative);

    /*!
     * Allows the user to select an option from the given list. The option is selected by typing the option
     * 
     * \param message The message to display
     * 
     * \param options The options to choose from
     * 
     * \param defaultOption The value to return if no option was selected
     * 
     * Returns the selected option
     */
    static int selectOption(std::string message, std::vector<int> options, int defaultOption);

    /*!
     * Allows the user to select an option from the given list. The option is selected by typing its index
     * 
     * \param message The message to display
     * 
     * \param options The options to choose from
     * 
     * \param defaultIndex The index to return if no option was choosen
     * 
     * Returns the index of the selected option
     */
    static unsigned int selectOptionIndex(std::string message, std::vector<int> options, unsigned int defaultIndex);
};

#endif	/* USERINPUT_H */


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

    static constexpr unsigned char INPUT_USE_DEFAULT = 0x1;
    static constexpr unsigned char INPUT_ALLOW_EMPTY = 0x2;
    static constexpr unsigned char INPUT_ALLOW_ZERO = 0x2;  //duplicate 0x2 on purpose!
    static constexpr unsigned char INPUT_ALLOW_NEGATIVE = 0x4;

    /*!
     * Prints a section-title in a common formatted way
     *
     * \param title The title to print
     */
    static void printSection(const std::string title);

    /*!
     * Allows the user to input a bool-value.
     * Accepted values are 'Y', 'Yes' and 'N', 'No' (lower and mixed case)
     *
     * \param message The message to display
     * \param defaultValue The default value for an empty input, only active in conjunction with INPUT_USE_DEFAULT
     * \param flags The flags (only INPUT_USE_DEFAULT)
     *
     * Returns the input bool-value
     */
    static bool inputBoolean(const std::string message, const bool defaultValue = false, const unsigned char flags = 0);

    /*!
     * Allows the user to input a line of text
     *
     * \param message The message to display
     * \param defaultValue The default value to use an empty string is input, only active in conjunction with INPUT_USE_DEFAULT
     * \param flags The flags (INPUT_USE_DEFAULT, INPUT_ALLOW_EMPTY) INPUT_ALLOW_EMPTY takes precedence
     *
     * Returns the input line as string
     */
    static std::string inputString(const std::string message, const std::string defaultValue = "", const unsigned char flags = 0);

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
    static std::string selectOption(const std::string message, const std::vector<std::string> options, const std::string defaultOption);

    /*!
     * Allows the user to select an option from the given list. The option is selected by typing its index
     *
     * \param message The message to display
     *
     * \param options The options to choose from
     *
     * \param defaultIndex The default index to select if no existing index was chosen
     *
     * Returns the selected index
     */
    static unsigned int selectOptionIndex(const std::string message, const std::vector<std::string> options, unsigned int defaultIndex);

    /*!
     * Allows the user to input a integer number.
     *
     * \param message The message to display
     * \param defaultValue The default value for an invalid input, only used in conjunction with INPUT_USE_DEFAULT
     * \param flags The flags (INPUT_USE_DEFAULT, INPUT_ALLOW_ZERO, INPUT_ALLOW_NEGATIVE)
     *
     * Returns the number typed
     */
    static int inputNumber(const std::string message, const int defaultValue = 0, const unsigned char flags = 0);

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
    static int selectOption(const std::string message, const std::vector<int> options, int defaultOption);

    /*!
     * Allows the user to select an option from the given list. The option is selected by typing its index
     *
     * \param message The message to display
     *
     * \param options The options to choose from
     *
     * \param defaultIndex The index to return if no option was chosen
     *
     * Returns the index of the selected option
     */
    static unsigned int selectOptionIndex(const std::string message, const std::vector<int> options, unsigned int defaultIndex);
};

#endif	/* USERINPUT_H */


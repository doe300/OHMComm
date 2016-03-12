
#include "UserInput.h"

using namespace std;
using namespace ohmcomm;

inline void clearInput()
{
    cin.clear();
    cin.ignore(INT_MAX, '\n');
}
template<typename T>
inline void printVector(const vector<T> v, bool withIndex)
{
    if(!withIndex)
    {
        cout << "(";
    }
    for (unsigned int i = 0; i < v.size(); i++)
    {
        if(withIndex)
        {
            cout << endl << i << ": ";
        }
        else if(i > 0)
        {
            cout << ", ";
        }
        cout << v[i];

    }
    if(!withIndex)
    {
        cout << ")";
    }
    else
    {
        cout << endl;
    }
}

void UserInput::printSection(const std::string title)
{
    std::cout << std::endl;
    std::cout << "+++ " << title << " +++" << std::endl;
}

bool UserInput::inputBoolean(const std::string message, const bool defaultValue, const unsigned char flags)
{
    cout << message << " (Yes/No)";
    if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
    {
        cout << (defaultValue ? " [Yes]" : " [No]");
    }
    cout << ": ";
    string result;
    cin >> result;
    if(result == "Y" || result== "Yes" || result == "y" || result == "yes")
    {
        return true;
    }
    if(result == "N" || result== "No" || result == "n" || result == "no")
    {
        return false;
    }
    if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
    {
        return defaultValue;
    }
    cout << "Invalid input: Please type 'Yes' or 'No'!" << endl;
    return inputBoolean(message);
}

std::string UserInput::inputString(const std::string message, const std::string defaultValue, const unsigned char flags)
{
    cout << message;
    if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
    {
        cout << " [" << defaultValue << "]";
    }
    cout << ": ";
    string result;
    cin >> result;
    if(result.empty())
    {
        if((flags & INPUT_ALLOW_EMPTY) == INPUT_ALLOW_EMPTY)
            return "";
        if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
            return defaultValue;
        //else rerun input
        std::cout << "Invalid input: Empty string is not allowed!" << std::endl;
        return UserInput::inputString(message, defaultValue, flags);
    }
    return result;
}

std::string UserInput::selectOption(const std::string message, const std::vector<std::string> options, const std::string defaultOption)
{
    cout << message;
    printVector(options, false);
    cout << " [" << defaultOption << "]";
    cout << ": ";
    string result;
    cin >> result;
    for (size_t i = 0; i < options.size(); i++)
    {
        if(result == options[i])
        {
            return options[i];
        }
    }
    return defaultOption;
}

unsigned int UserInput::selectOptionIndex(const std::string message, const std::vector<std::string> options, unsigned int defaultIndex)
{
    cout << message;
    cout << " [" << defaultIndex << "]";
    cout << ": ";
    printVector(options, true);
    unsigned int selectedIndex;
    if(!(cin >> selectedIndex))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return selectOptionIndex(message, options, defaultIndex);
    }
    if(selectedIndex < options.size())
    {
        return selectedIndex;
    }
    return defaultIndex;
}

int UserInput::inputNumber(const std::string message, const int defaultValue, const unsigned char flags)
{
    cout << message;
    if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
    {
        cout << " [" << defaultValue << "]";
    }
    cout << ": ";
    int result;
    if(!(cin >> result))
    {
        clearInput();
        if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
            return defaultValue;
        cout << "Invalid input: You must input a number!" << endl;
        return inputNumber(message, defaultValue, flags);
    }
    if(result == 0 && (flags & INPUT_ALLOW_ZERO) != INPUT_ALLOW_ZERO)
    {
        if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
            return defaultValue;
        cout << "Invalid input: zero is not allowed!" << endl;
        return inputNumber(message, defaultValue, flags);
    }
    if(result < 0 && (flags & INPUT_ALLOW_NEGATIVE) != INPUT_ALLOW_NEGATIVE)
    {
        if((flags & INPUT_USE_DEFAULT) == INPUT_USE_DEFAULT)
            return defaultValue;
        cout << "Invalid input: only positive numbers are allowed!" << endl;
        return inputNumber(message, defaultValue, flags);
    }
    return result;
}

int UserInput::selectOption(const std::string message, const std::vector<int> options, int defaultOption)
{
    cout << message;
    printVector(options, false);
    cout << " [" << defaultOption << "]";
    cout << ": ";
    int result;
    if(!(cin >> result))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return selectOption(message, options, defaultOption);
    }
    for (size_t i = 0; i < options.size(); i++)
    {
        if(result == options[i])
        {
            return options[i];
        }
    }
    return defaultOption;
}

unsigned int UserInput::selectOptionIndex(const std::string message, const std::vector<int> options, unsigned int defaultIndex)
{
    cout << message;
    cout << " [" << defaultIndex << "]";
    cout << ": ";
    printVector(options, true);
    unsigned int selectedIndex;
    if(!(cin >> selectedIndex))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return selectOptionIndex(message, options, defaultIndex);
    }
    if(selectedIndex < options.size())
    {
        return selectedIndex;
    }
    return defaultIndex;
}

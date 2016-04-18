/* 
 * File:   Logger.cpp
 * Author: daniel
 * 
 * Created on March 23, 2016, 2:55 PM
 */

#include "Logger.h"
#include "OHMComm.h"

using namespace ohmcomm;

std::unique_ptr<Logger> Logger::LOGGER = std::unique_ptr<Logger>(new ConsoleLogger());

std::wostream& ConsoleLogger::write(const LogLevel level)
{
    //select correct stream and set colors
    if(level == ERROR)
        return std::wcerr << "\033[31m";
    if(level == WARNING)
        return std::wcerr << "\033[33m";
    if(level == DEBUG)
        return std::wcerr << "\033[37m";
    return std::wcout;
}

std::wostream& ConsoleLogger::end(std::wostream& stream)
{
    //reset colors and print EOL
    return stream << "\033[39;49m" << std::endl;
}
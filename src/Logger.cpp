/* 
 * File:   Logger.cpp
 * Author: daniel
 * 
 * Created on March 23, 2016, 2:55 PM
 */

#include "Logger.h"
#include "OHMComm.h"

using namespace ohmcomm;

std::unique_ptr<Logger> Logger::LOGGER = std::unique_ptr<Logger>(new DefaultLogger());

std::wostream& DefaultLogger::write(const LogLevel level)
{
    if(level >= WARNING)
        return std::wcerr;
    return std::wcout;
}

std::wostream& DefaultLogger::end(std::wostream& stream)
{
    return std::endl(stream);
}
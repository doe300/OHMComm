/* 
 * File:   Logger.h
 * Author: daniel
 *
 * Created on March 23, 2016, 2:55 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <memory>
#include <string>
#include <iostream>

//To fix an error with MSVC and the LogLevel::ERROR constant
#undef ERROR

namespace ohmcomm
{

    /*!
     * Simple wrapper for different logging frameworks
     * 
     * \since 0.9
     */
    class Logger
    {
    public:
        
        enum LogLevel : unsigned char
        {
            DEBUG = 0,
            INFO = 1,
            WARNING = 2,
            ERROR = 3
        };
        
        /*!
         * The currently active Logger, change this to redirect logging
         */
        static std::unique_ptr<Logger> LOGGER;
        
        /*!
         * \param level The level to log
         * \return the output stream for the given level
         */
        virtual std::wostream& write(const LogLevel level) = 0;
        
        /*!
         * Ends the "line" of log and flushes the stream, if necessary
         */
        virtual std::wostream& end(std::wostream& stream) = 0;
        
        Logger() = default;
        Logger(const Logger& orig) = default;
        virtual ~Logger() = default;
    };
   
    /*!
     * Default logger, writing everything to std::wcout or std::wcerr
     * 
     * \since 0.9
     */
    class ConsoleLogger : public Logger
    {
        virtual std::wostream& write(const LogLevel level) override;
        virtual std::wostream& end(std::wostream& stream) override;
    };
    
    
    //For simpler/shorter access to logger
    
    template<Logger::LogLevel level>
    inline std::wostream& log(const char* component)
    {
        return Logger::LOGGER->write(level) << '[' << component << "] ";
    }
    
    static const auto debug = log<Logger::DEBUG>;
    static const auto info = log<Logger::INFO>;
    static const auto warn = log<Logger::WARNING>;
    static const auto error = log<Logger::ERROR>;
    
    inline std::wostream& endl(std::wostream& stream)
    {
        return Logger::LOGGER->end(stream);
    }
    
    /*!
     * Allows writing std::strings into a wstring-stream
     */
    inline std::wostream& operator <<(std::wostream& stream, const std::string& string)
    {
        return stream << string.data();
    }
}

#endif	/* LOGGER_H */


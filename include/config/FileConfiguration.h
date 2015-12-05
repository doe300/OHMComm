/* 
 * File:   FileConfiguration.h
 * Author: daniel
 *
 * Created on November 30, 2015, 5:36 PM
 */

#ifndef FILECONFIGURATION_H
#define	FILECONFIGURATION_H

#include "ConfigurationMode.h"
#include "Utility.h"
#include <map>
#include <fstream>

/*!
 * File-based configuration.
 *
 * The format for the configuration-file is following:
 *
 * - Lines starting with a '#' are ignored as comments
 * - Every other non-empty line is interpreted as key-value pair
 * - the key is an arbitrary string of digits (0-9), letters (a-z, A-Z), underscore (_) and minus (-)
 *      Note: any other character may occur, but interpretation is undefined
 * - the value is one of the following:
 *      - a number (as understood by atoi())
 *      - a boolean value ('true' or 'false'). Booleans can also be represented as numbers 0 for false, any other number for true
 *      - a string (arbitrary list of characters enclosed by double-quotes '"')
 *
 * The names of the predefined configuration-keys are taken from the existing Parameter-constants in Parameters
 */
class FileConfiguration : public ConfigurationMode
{
public:
    FileConfiguration(const std::string fileName);

    virtual bool runConfiguration();

    const std::string getCustomConfiguration(const std::string key, const std::string message, const std::string defaultValue) const;
    const int getCustomConfiguration(const std::string key, const std::string message, const int defaultValue) const;
    const bool getCustomConfiguration(const std::string key, const std::string message, const bool defaultValue) const;
    const bool isCustomConfigurationSet(const std::string key, const std::string message) const;

private:
    const std::string configFile;
    std::map<std::string, std::string> customConfig;
};

#endif	/* FILECONFIGURATION_H */


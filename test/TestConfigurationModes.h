#ifndef TESTCONFIGURATIONMODES_H
#define TESTCONFIGURATIONMODES_H

#include "ConfigurationMode.h"
#include "cpptest.h"

class TestConfigurationModes : public Test::Suite
{
public:
    TestConfigurationModes();

    void testParameterConfiguration();

    void testInteractiveConfiguration();

    void testLibraryConfiguration();

    void testPassiveConfiguration();
    
    void testFileConfiguration();
};

#endif // TESTCONFIGURATIONMODES_H

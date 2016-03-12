#ifndef TESTCONFIGURATIONMODES_H
#define TESTCONFIGURATIONMODES_H

#include "config/ConfigurationMode.h"
#include "config/InteractiveConfiguration.h"
#include "config/LibraryConfiguration.h"
#include "config/ParameterConfiguration.h"
#include "config/PassiveConfiguration.h"
#include "sip/SIPConfiguration.h"
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
    
    void testSIPConfiguration();
};

#endif // TESTCONFIGURATIONMODES_H

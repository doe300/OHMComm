/* 
 * File:   TestSIPGrammar.cpp
 * Author: daniel
 * 
 * Created on February 13, 2016, 12:07 PM
 */

#include "TestSIPGrammar.h"

TestSIPGrammar::TestSIPGrammar() : Suite()
{
    TEST_ADD(TestSIPGrammar::testSIPURI);
    TEST_ADD(TestSIPGrammar::testNamedAddress);
    TEST_ADD(TestSIPGrammar::testToViaAddress);
}

void TestSIPGrammar::testSIPURI()
{
    const std::string fullURI("sips:user:secret@host.com:56;transport=udp;method=INVITE;maddr=3;ttl=70;lr;attr=value;prop=val;user=phone?h1=v1&h2=v2&h3=v4");
    const std::string shortURI("sip:host.com");
    
    const SIPGrammar::SIPURI fullSIPURI = SIPGrammar::readSIPURI(fullURI, 100);
    const SIPGrammar::SIPURI shortSIPURI = SIPGrammar::readSIPURI(shortURI, -1);
    
    //assert correct reading
    TEST_ASSERT(!fullSIPURI.host.empty());
    TEST_ASSERT(!shortSIPURI.host.empty());
    
    const std::string fullGeneratedURI = SIPGrammar::toSIPURI(fullSIPURI);
    const std::string shortGeneratedURI = SIPGrammar::toSIPURI(shortSIPURI);
    
    //assert correct generation
    TEST_ASSERT(fullURI.compare(fullGeneratedURI) == 0);
    TEST_ASSERT(shortURI.compare(shortGeneratedURI) == 0);
}

void TestSIPGrammar::testNamedAddress()
{
    const std::string withName("\"Jane Doe\" <sip:user:secret@host.com:100>");
    const std::string withoutName("<sip:host.com:100>");
    
    const auto addressWithName = SIPGrammar::readNamedAddress(withName, 100);
    const auto addressWithoutName = SIPGrammar::readNamedAddress(withoutName, 100);
    
    //assert correct reading
    TEST_ASSERT(!std::get<0>(addressWithName).empty());
    TEST_ASSERT(std::get<0>(addressWithoutName).empty());
    
    const std::string withNameGenerated = SIPGrammar::toNamedAddress(std::get<1>(addressWithName), std::get<0>(addressWithName));
    const std::string withoutNameGenerated = SIPGrammar::toNamedAddress(std::get<1>(addressWithoutName), std::get<0>(addressWithoutName));
    
    //assert correct generation
    TEST_ASSERT(withName.compare(withNameGenerated) == 0);
    TEST_ASSERT(withoutName.compare(withoutNameGenerated) == 0);
}

void TestSIPGrammar::testToViaAddress()
{
    const std::string viaAddress("SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8;received=192.0.2.1");
    
    const auto readAddress = SIPGrammar::readViaAddress(viaAddress, 0);
    
    TEST_ASSERT(std::get<0>(readAddress).compare("SIP/2.0/UDP") == 0);
    TEST_ASSERT(std::get<1>(readAddress).host.compare("pc33.atlanta.com") == 0);
    
    const std::string generatedAddress = SIPGrammar::toViaAddress(std::get<1>(readAddress), std::get<0>(readAddress));

    TEST_ASSERT(viaAddress.compare(generatedAddress) == 0);
}

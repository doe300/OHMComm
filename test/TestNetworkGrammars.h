/* 
 * File:   TestNetworkGrammars.h
 * Author: daniel
 *
 * Created on February 13, 2016, 1:05 PM
 */

#ifndef TESTNETWORKGRAMMARS_H
#define	TESTNETWORKGRAMMARS_H

#include "cpptest.h"

#include "network/NetworkGrammars.h"

class TestNetworkGrammars : public Test::Suite
{
public:
    TestNetworkGrammars();

    void testToHostAndPort();
    
    void testValidHost();
    
    void testToPort();
};

#endif	/* TESTNETWORKGRAMMARS_H */


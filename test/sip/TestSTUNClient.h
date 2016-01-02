/* 
 * File:   TestSTUNClient.h
 * Author: daniel
 *
 * Created on January 2, 2016, 2:00 PM
 */

#ifndef TESTSTUNCLIENT_H
#define	TESTSTUNCLIENT_H

#include "cpptest.h"

class TestSTUNClient : public Test::Suite
{
public:
    TestSTUNClient();

    void testSTUNRequest();
};

#endif	/* TESTSTUNCLIENT_H */


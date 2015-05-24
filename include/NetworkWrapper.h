#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <iostream>
#include <string>

class NetworkWrapper
{
public:
	virtual void sendDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0) = 0;
	virtual void recvDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0) = 0;
};

#endif
#ifndef NETWORKWRAPPER_H
#define	NETWORKWRAPPER_H

#include <iostream>

//Socket-ID for an invalid socket
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
//define SOCKET_ERROR for non-Windows
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

class NetworkWrapper
{
public:
	virtual void sendDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0) = 0;
	virtual void recvDataNetworkWrapper(void *buffer, unsigned int bufferSize = 0) = 0;
        
        /*!
         * Static method for retrieving the last error code
         */
        static int getLastError();
        
        /*!
         * Static method for closing socket platform independent
         */
        static void closeSocket(int fd);
};

#endif
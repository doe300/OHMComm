#include "NetworkWrapper.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h> // close()
#include <unistd.h>
#endif

int NetworkWrapper::getLastError()
{
	int error;

	#ifdef _WIN32
	error = WSAGetLastError();
	#else
	error = errno;
	#endif

	return error;
}

void NetworkWrapper::closeSocket(int fd)
{
    shutdown(fd, SHUT_RDWR);
    #ifdef _WIN32
    closesocket(fd);
    #else
    close(fd);
    #endif
}

#include "NetworkWrapper.h"

#ifdef _WIN32
#include <winsock2.h>
#define SHUTDOWN_BOTH SD_BOTH   // 2
#else
#include <sys/socket.h> // close()
#include <unistd.h>
#define SHUTDOWN_BOTH SHUT_RDWR // 2
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
    shutdown(fd, SHUTDOWN_BOTH);
    #ifdef _WIN32
    closesocket(fd);
    #else
    close(fd);
    #endif
}

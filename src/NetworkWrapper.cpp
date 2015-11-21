#include <algorithm>

#include "NetworkWrapper.h"

bool NetworkWrapper::isIPv6(const std::string ipAddress)
{
    if(ipAddress.find(':') != std::string::npos)
    {
        //at least one ':' in the address
        //because some ':' can be discarded, we can't check for exactly 7 ':'
        //so we simply accept if one occurs
        return true;
    }
    return false;
}

bool NetworkWrapper::hasTimedOut() const
{
    int error;
    #ifdef _WIN32
    error = WSAGetLastError();
    #else
    error = errno;
    #endif

    return error == EAGAIN || error == EWOULDBLOCK || error == WSAETIMEDOUT;
}

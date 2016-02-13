#include <algorithm>

#include "network/NetworkWrapper.h"

std::wstring NetworkWrapper::getLastError() const
{
    int error;
#ifdef _WIN32
    error = WSAGetLastError();
    wchar_t* tmp;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, LANG_USER_DEFAULT, (wchar_t*) & tmp, 0, nullptr);
#else
    error = errno;
    wchar_t tmp[255];
    char* errPtr = strerror(error);
    mbstowcs(tmp, errPtr, 255);
#endif
    return (std::to_wstring(error) + L" - ") +tmp;
}

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

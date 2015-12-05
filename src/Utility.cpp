/* 
 * File:   Utility.cpp
 * Author: daniel
 * 
 * Created on December 5, 2015, 4:14 PM
 */

#include "Utility.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#endif

const unsigned int MAX_NAME_SIZE{ 255 };

std::string Utility::getDomainName()
{
    char hostName[MAX_NAME_SIZE];
    int status = gethostname(hostName, MAX_NAME_SIZE);
    if (status == 0)
    {
        return std::string(hostName);
    }
    return std::string("unknown");
}

std::string Utility::getUserName()
{
#ifdef _WIN32
    char userName[MAX_NAME_SIZE];
    DWORD nameSize = sizeof(userName);
    int status = GetUserName((LPTSTR)userName, &nameSize);
    if (status == 0)
    {
        return std::string(userName);
    }
#else
    uid_t userID = getuid();
    struct passwd *userInfo;
    userInfo = getpwuid(userID);
    if (userInfo != nullptr)
    {
        return std::string(userInfo->pw_name);
    }
#endif
	return std::string("unknown");
}

std::string Utility::trim(const std::string& in)
{
    //https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    auto wsfront=std::find_if_not(in.begin(),in.end(),[](int c){return std::isspace(c);});
    return std::string(wsfront,std::find_if_not(in.rbegin(),std::string::const_reverse_iterator(wsfront),[](int c){return std::isspace(c);}).base());
}

bool Utility::compareIgnoreCase(const std::string& s1, const std::string s2)
{
    if(s1.size() != s2.size())
    {
        return false;
    }
#ifdef _WIN32
    return lstrcmpi(s1.data(), s2.data()) == 0;
#else
    return strcasecmp(s1.data(), s2.data()) == 0;
#endif
}

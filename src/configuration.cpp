/*
 * File:   ConfigurationMode.cpp
 * Author: daniel
 *
 * Created on August 19, 2015, 4:48 PM
 */

#include "configuration.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

const unsigned int MAX_NAME_SIZE{ 255 };

std::string getDomainName()
{
	char hostName[MAX_NAME_SIZE];
	int status = gethostname(hostName, MAX_NAME_SIZE);
	if (status == 0)
	{
		return std::string(hostName);
	}
	return std::string("unknown");
}

std::string getUserName()
{
	char userName[MAX_NAME_SIZE];
#ifdef _WIN32
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


#include "CryptoSuite.h"

#ifdef OPENSSL_CRYPTO_LIBRARY //Only compile of OpenSSL crypto is available

#include <openssl/md5.h>

using namespace ohmcomm;

const bool CryptoSuite::CRYPTO_SUITE_AVAILABLE = true;

CryptoSuite::CryptoSuite()
{
    
}

CryptoSuite::~CryptoSuite()
{
    
}

std::string CryptoSuite::hashMD5(const std::string& input)
{
    char result[MD5_DIGEST_LENGTH];
    MD5_CTX data;
    MD5_Init(&data);
    MD5_Update(&data, input.data(), input.size());
    MD5_Final((unsigned char*)&result, &data);
    return result;
}

#endif
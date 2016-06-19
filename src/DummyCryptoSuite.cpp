#include "CryptoSuite.h"

#if !defined CRYPTOPP_LIBRARY && !defined OPENSSL_CRYPTO_LIBRARY

#include <stdexcept>

using namespace ohmcomm;

const bool CryptoSuite::CRYPTO_SUITE_AVAILABLE = false;

CryptoSuite::CryptoSuite()
{
}

CryptoSuite::~CryptoSuite()
{
    
}

std::string CryptoSuite::hashMD5(const std::string& input)
{
    throw std::runtime_error("No crypto library configured!");
}
#endif
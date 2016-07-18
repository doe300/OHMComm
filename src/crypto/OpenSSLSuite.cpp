
#include "crypto/CryptoSuite.h"

#ifdef OPENSSL_CRYPTO_LIBRARY //Only compile of OpenSSL crypto is available

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <string.h>

#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

using namespace ohmcomm::crypto;

const bool CryptoSuite::CRYPTO_SUITE_AVAILABLE = true;
const std::string CryptoSuite::CRYPTO_SUITE_NAME = OPENSSL_VERSION_TEXT;

std::string CryptoSuite::hashMD5(const std::string& input)
{
    byte digest[MD5_DIGEST_LENGTH];
    MD5((const byte*)input.data(), input.size(), digest);
    std::stringstream stream;
    for(unsigned int i = 0; i < sizeof(digest); ++i)
    {
        stream << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << (int)digest[i];
    }
    return stream.str();
}

std::vector<byte> CryptoSuite::createHMAC_SHA1(const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    byte digest[SHA_DIGEST_LENGTH];
    const EVP_MD* algorithm = EVP_sha1();
    HMAC(algorithm, cryptoContext->masterKey.data(), cryptoContext->masterKey.size(), (const byte*)buffer, bufferSize, digest, nullptr);
    std::vector<byte> result(SHA_DIGEST_LENGTH);
    std::copy(digest, digest + SHA_DIGEST_LENGTH, result.data());
    return result;
}

bool CryptoSuite::verifyHMAC_SHA1(const void* hmacBuffer, const unsigned int hmacBufferSize, const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext)
{
    //TODO is there a better way to verify??
    const std::vector<byte> compare = createHMAC_SHA1(buffer, bufferSize, cryptoContext);
    return memcmp(compare.data(), hmacBuffer, hmacBufferSize) == 0;
}

std::vector<byte> CryptoSuite::encryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<byte>& key)
{
    const EVP_CIPHER* cipher = nullptr;
    switch(mode)
    {
        case CipherMode::COUNTER_MODE:
            cipher = EVP_aes_128_ctr();
            break;
        case CipherMode::F8:
            cipher = EVP_aes_128_cfb8();
            break;
    default:
        throw std::invalid_argument("Unknown cipher-mode!");
    }
    
    //additional space for padding
    const unsigned int resultSize = (bufferSize / cipher->block_size + 1) * cipher->block_size;
    std::vector<byte>result(resultSize, '\0');
    
    EVP_CIPHER_CTX data;
    EVP_CIPHER_CTX_init(&data);
    EVP_EncryptInit(&data, cipher, key.data(), nullptr);
    unsigned int bytesWritten = 0;
    int tmp = 0;
    while(bytesWritten < bufferSize)
    {
        EVP_EncryptUpdate(&data, result.data() + bytesWritten, &tmp, (const byte*)inputBuffer + bytesWritten, bufferSize);
        bytesWritten += tmp;
    }
    EVP_EncryptFinal(&data, result.data() + bytesWritten, &tmp);
    bytesWritten += tmp;
    EVP_CIPHER_CTX_cleanup(&data);
    
    result.resize(bytesWritten);
    return result;
}

std::vector<byte> CryptoSuite::decryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<byte>& key)
{
    std::vector<byte>result(bufferSize);
    const EVP_CIPHER* cipher = nullptr;
    switch(mode)
    {
        case CipherMode::COUNTER_MODE:
            cipher = EVP_aes_128_ctr();
            break;
        case CipherMode::F8:
            cipher = EVP_aes_128_cfb8();
            break;
    default:
        throw std::invalid_argument("Unknown cipher-mode!");
    }
    EVP_CIPHER_CTX data;
    EVP_CIPHER_CTX_init(&data);
    EVP_DecryptInit(&data, cipher, key.data(), nullptr);
    unsigned int bytesWritten = 0;
    int tmp = 0;
    while(bytesWritten < bufferSize)
    {
        EVP_DecryptUpdate(&data, result.data() + bytesWritten, &tmp, (const byte*)inputBuffer + bytesWritten, bufferSize);
        bytesWritten += tmp;
    }
    EVP_DecryptFinal(&data, result.data() + bytesWritten, &tmp);
    EVP_CIPHER_CTX_cleanup(&data);
    return result;
}

#endif
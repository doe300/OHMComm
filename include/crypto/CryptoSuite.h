/* 
 * File:   CryptoSuite.h
 * Author: doe300
 *
 * Created on June 17, 2016, 5:47 PM
 */

#ifndef CRYPTOSUITE_H
#define CRYPTOSUITE_H

#include <string>
#include <memory>

#include "CryptographicContext.h"

namespace ohmcomm
{
    namespace crypto
    {

        /*!
         * Maps cryptographic and hashing-functions to an underlying API
         * 
         * \since 0.9
         */
        class CryptoSuite
        {
        public:

            static const bool CRYPTO_SUITE_AVAILABLE;

            CryptoSuite();
            ~CryptoSuite();

            /*!
             * Calculates the MD5 hash of the given input-string
             * 
             * \return the MD5 hash
             */
            static std::string hashMD5(const std::string& input);

            /*!
             * Creates a HMAC SHA1 digest for the given data
             * 
             * \param buffer The buffer containg the data to hash
             * 
             * \param bufferSize The number of bytes in the buffer
             * 
             * \param cryptoContext The container for the master key to use
             * 
             * \return A vector containing the resulting digest
             */
            static std::vector<unsigned char> createHMAC_SHA1(const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext);

            /*!
             * Verifies an HMAC SHA1 digest for given data
             * 
             * \param hmacBuffer The buffer containing the HMAC digest
             * 
             * \param hmacBufferSize The size (in bytes) of the HMAC buffer
             * 
             * \param buffer The buffer containing the raw data to verify against
             * 
             * \param bufferSize The number of bytes of raw data
             * 
             * \param cryptoContext The container for the master key to use
             * 
             * \return Whether the digest could be verified
             */
            static bool verifyHMAC_SHA1(const void* hmacBuffer, const unsigned int hmacBufferSize, const void* buffer, const unsigned int bufferSize, const std::shared_ptr<CryptographicContext> cryptoContext);
            
            static std::vector<unsigned char> encryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<unsigned char> key);
            
            static std::vector<unsigned char> decryptAES(const ohmcomm::crypto::CipherMode mode, const void* inputBuffer, const unsigned int bufferSize, const std::vector<unsigned char> key);
            
        private:
            CryptoSuite(const CryptoSuite& orig) = delete;
        };
    }
}
#endif /* CRYPTOSUITE_H */


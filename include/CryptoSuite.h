/* 
 * File:   CryptoSuite.h
 * Author: doe300
 *
 * Created on June 17, 2016, 5:47 PM
 */

#ifndef CRYPTOSUITE_H
#define CRYPTOSUITE_H

#include <string>

namespace ohmcomm
{

    class CryptoSuite
    {
    public:
        
        static const bool CRYPTO_SUITE_AVAILABLE;
        
        CryptoSuite();
        ~CryptoSuite();
        
        static std::string hashMD5(const std::string& input);
    private:
        CryptoSuite(const CryptoSuite& orig) = delete;
    };
}
#endif /* CRYPTOSUITE_H */


/* 
 * File:   STUNClient.h
 * Author: daniel
 *
 * Created on January 1, 2016, 12:50 PM
 */

#ifndef STUNCLIENT_H
#define	STUNCLIENT_H

#include <vector>
#include <tuple>
#include <memory>
#include <map>

#include "Utility.h"

namespace ohmcomm
{
    namespace sip
    {

        /*!
         * STUN client for RFC 5389
         * 
         * NOTE: this implementation is not thread-safe!
         * 
         * See: https://tools.ietf.org/html/rfc5389
         */
        class STUNClient
        {
        public:

            STUNClient();

            ~STUNClient();

            /*!
             * Tries to retrieve the external IP-address and port of this device by contacting a set of predefined STUN-server
             * 
             \return a tuple containing of following values:
             *      a boolean flag to determine, whether one of the requests was successful
             *      a string containing the determined IP-address
             *      a number containing the determined port
             */
            const std::tuple<bool, std::string, unsigned short> retrieveSIPInfo();

        private:
            typedef uint16_t STUNMessageType;

            static constexpr STUNMessageType STUN_BINDING_REQUEST = 0x0001;
            static constexpr STUNMessageType STUN_BINDING_RESPONSE = 0x0101;
            static constexpr STUNMessageType STUN_BINDING_ERROR_RESPONSE = 0x0111;
            //list is not exhaustive (see: https://www.iana.org/assignments/stun-parameters/stun-parameters.txt)

            typedef uint16_t STUNAttributeType;

            static constexpr STUNAttributeType STUN_INVALID_ATTRIBUTE = 0x0000;
            static constexpr STUNAttributeType STUN_MAPPED_ADDRESS = 0x0001;
            static constexpr STUNAttributeType STUN_CHANGE_REQUEST = 0x0003;
            static constexpr STUNAttributeType STUN_USERNAME = 0x0006;
            static constexpr STUNAttributeType STUN_MESSAGE_INTEGRITY = 0x0008;
            static constexpr STUNAttributeType STUN_ERROR_CODE = 0x0009;
            static constexpr STUNAttributeType STUN_UNKNOWN_ATTRIBUTES = 0x000A;
            static constexpr STUNAttributeType STUN_XOR_MAPPED_ADDRESS = 0x8020;
            static constexpr STUNAttributeType STUN_ALTERNATE_SERVER = 0x8023;
            //list is not exhaustive (see: https://www.iana.org/assignments/stun-parameters/stun-parameters.txt)

            static constexpr uint16_t MAPPED_ADDRESS_IPv4 = 0x1;
            static constexpr uint16_t MAPPED_ADDRESS_IPv6 = 0x2;

            static const std::vector<std::string> STUN_SERVERS;
            static constexpr unsigned short DEFAULT_STUN_PORT{3478};
            static const std::string DEFAULT_SOURCE_IP;
            static constexpr unsigned short LOCAL_PORT{54320};
            static constexpr uint32_t MAGIC_COOKIE{0x2112A442};
            static constexpr unsigned short BUFFER_SIZE{2048};
            static constexpr uint8_t STUN_HEADER_SIZE{20};
            static constexpr uint8_t MAX_RETRIES{8};
            char buffer[2048];
            std::string transactionID;

            struct STUNAttribute
            {
                STUNAttributeType type;
                const char* valuePointer;
                unsigned int valueLength;
            };

            /*!
             * Tries to connect to a STUN server and returns the response
             * 
             * \param stunServer The host-name or IP-address of the STUN-server
             * 
             * \param serverPort The port of the STUN-server to access
             * 
             * \return a tuple containing of following values:
             *      a boolean flag to determine, whether our request was successful
             *      a string containing the determined IP-address
             *      a number containing the determined port
             */
            const std::tuple<bool, std::string, unsigned short> testSTUNServer(const std::string& stunServer, unsigned short serverPort = DEFAULT_STUN_PORT);

            /*!
             * Creates a request for the given STUN message-type and content into the local buffer
             * 
             * \param type The STUNMessageType
             * 
             * \param messageBody The attributes of this message
             * 
             * \return the number of bytes filled in the buffer
             */
            unsigned int createRequestMessage(STUNMessageType type, const std::string& messageBody = "");

            /*!
             * Generates a random hexadecimal number with 96-bit (12 Byte) of data into the given position
             */
            void createTransactionID(char* position);

            /*!
             * Reads a single attribute from the internal buffer
             * 
             * \param maxSize the maximum size if the body
             * 
             * \param position The position (in bytes) in the buffer to start reading
             * 
             * \return a tuple containing of the attribute-type (or STUN_INVALID_ATTRIBUTE if no more attributes exist),
             * the starting address of the attribute-specific value and the number of bytes, the attribute-value uses (the attribute-length)
             */
            const STUNAttribute readAttribute(unsigned int maxSize, unsigned int position = STUN_HEADER_SIZE) const;

            /*!
             * Reads the address for the MAPPED-ADDRESS or ALTERNATE-SERVER attribute
             * 
             * \param attribute The attribute-info to read the address from
             * 
             * \param magicCookie The magicCookie to XOR for XOR-MAPPED-ADDRESS
             * 
             * \return a tuple containing the IP-address and port
             */
            const std::tuple<std::string, unsigned short> readMappedAddress(const STUNAttribute& attribute, const uint32_t magicCookie);
        };
    }
}
#endif	/* STUNCLIENT_H */


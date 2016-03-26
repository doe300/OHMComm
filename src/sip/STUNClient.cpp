/* 
 * File:   STUNClient.cpp
 * Author: daniel
 * 
 * Created on January 1, 2016, 12:50 PM
 */

#include "sip/STUNClient.h"
#include "network/UDPWrapper.h"

#include <chrono>

using namespace ohmcomm::sip;

const std::vector<std::string> STUNClient::STUN_SERVERS = {
    "stun.stunprotocol.org",
    "stun.sipgate.net",
    //"stun.l.google.com:19302",
    "stun01.sipphone.com",
    "stunserver.org"
};

const std::string STUNClient::DEFAULT_SOURCE_IP{"0.0.0.0"};

STUNClient::STUNClient()
{
}

STUNClient::~STUNClient()
{
}

const std::tuple<bool, std::string, unsigned short> STUNClient::retrieveSIPInfo()
{
    std::tuple<bool, std::string, unsigned short> result = std::make_tuple(false, "", 0);
    unsigned short index = 0;
    while(index < STUN_SERVERS.size() && std::get<0>(result) == false)
    {
        const std::string serverIP = Utility::getAddressForHostName(STUN_SERVERS[index]);
        if(!serverIP.empty())
        {
            result = testSTUNServer(serverIP);
        }
        ++index;
    }
    std::cout << "STUN: Found result: " << std::get<1>(result) << ':' << std::get<2>(result) << std::endl;
    return result;
}

const std::tuple<bool, std::string, unsigned short> STUNClient::testSTUNServer(const std::string& stunServer, unsigned short serverPort)
{
    ohmcomm::network::UDPWrapper network(LOCAL_PORT, stunServer, serverPort);
 
    std::cout << "STUN: Testing ... " << stunServer << ':' << serverPort << std::endl;
    const unsigned int sendSize = createRequestMessage(STUN_BINDING_REQUEST);
    int receivedSize = INVALID_SOCKET;
    uint8_t numRetries = MAX_RETRIES;
    do
    {
        //we re-send until we receive something (or run out of retries)
        network.sendData(buffer, sendSize);
        receivedSize = network.receiveData(buffer, BUFFER_SIZE).status;
        --numRetries;
    }
    while(numRetries > 0 && receivedSize == ohmcomm::network::UDPWrapper::RECEIVE_TIMEOUT);
    if (receivedSize != INVALID_SOCKET && receivedSize != ohmcomm::network::UDPWrapper::RECEIVE_TIMEOUT && receivedSize >= STUN_HEADER_SIZE)
    {
        //handle response
        //1. check response-type
        const uint16_t responseType = ntohs(*(reinterpret_cast<uint16_t*>(buffer)));
        if(responseType == STUN_BINDING_RESPONSE || responseType == STUN_BINDING_ERROR_RESPONSE)
        {
            //1.1 read body-length
            const uint16_t bodySize = ntohs(*(reinterpret_cast<uint16_t*>(buffer + 2)));
            //1.2 check magic cookie and transaction-ID
            const uint32_t magicCookie = ntohl(*(reinterpret_cast<uint32_t*>(buffer + 4)));
            const char* transID = (const char*)(buffer + 8);
            if(magicCookie != MAGIC_COOKIE)
            {
                std::cout << "STUN: Magic Cookie does not match!" << std::endl;
            }
            else if(transactionID.compare(transID) != 0)
            {
                std::cout << "STUN: Transaction-IDs do not match!" << std::endl;
            }
            else //2. check attributes
            {
                //2.0 read attributes
                unsigned int position = STUN_HEADER_SIZE;
                std::vector<STUNClient::STUNAttribute> attributes;
                auto attribute = readAttribute(bodySize, position);
                int mappedAddressIndex = -1, xorMappedAddressIndex = -1, alternateServerIndex = -1, errorCodeIndex = -1;
                while(attribute.type != STUN_INVALID_ATTRIBUTE)
                {
                    attributes.push_back(attribute);
                    position += 4 + attribute.valueLength;
                    
                    //attributes may occur more than once, but only the first needs to be processed
                    if(attribute.type == STUN_MAPPED_ADDRESS && mappedAddressIndex == -1)
                        mappedAddressIndex = attributes.size()-1;
                    if(attribute.type == STUN_XOR_MAPPED_ADDRESS && xorMappedAddressIndex == -1)
                        xorMappedAddressIndex = attributes.size()-1;
                    if(attribute.type == STUN_ALTERNATE_SERVER && alternateServerIndex == -1)
                        alternateServerIndex = attributes.size()-1;
                    if(attribute.type == STUN_ERROR_CODE && errorCodeIndex == -1)
                        errorCodeIndex = attributes.size()-1;
                    
                    attribute = readAttribute(bodySize, position);
                }
                //2.1. check MAPPED-ADDRESS
                if(mappedAddressIndex != -1 && attributes[mappedAddressIndex].valueLength > 0)
                {
                    std::tuple<std::string, unsigned short> address = readMappedAddress(attributes[mappedAddressIndex], magicCookie);
                    return std::make_tuple(true, std::get<0>(address), std::get<1>(address));
                }
                //2.2 check XOR-MAPPED-ADDRESS
                if(xorMappedAddressIndex != -1 && attributes[xorMappedAddressIndex].valueLength > 0)
                {
                    std::tuple<std::string, unsigned short> address = readMappedAddress(attributes[mappedAddressIndex], magicCookie);
                    return std::make_tuple(true, std::get<0>(address), std::get<1>(address));
                }
                //2.3 check ALTERNATE-SERVER
                if(alternateServerIndex != -1 && attributes[alternateServerIndex].valueLength > 0)
                {
                    //try to contact alternate server
                    const std::tuple<std::string, unsigned short> serverAddress = readMappedAddress(attributes[mappedAddressIndex], magicCookie);
                    return testSTUNServer(std::get<0>(serverAddress), std::get<1>(serverAddress));
                }
                //2.4 print info from ERROR-CODE
                if(errorCodeIndex != -1 && attributes[errorCodeIndex].valueLength > 0)
                {
                    const STUNAttribute& attribute = attributes[errorCodeIndex];
                    //the class (the hundreds from the error-code) are the least 3 bits of the third byte
                    uint8_t errorClass = *((uint8_t*)(attribute.valuePointer + 2)) & 0x7;
                    uint8_t errorNumber = *((uint8_t*)(attribute.valuePointer + 3));
                    uint16_t errorCode = errorClass * 100 + errorNumber;
                    std::string reason(attribute.valuePointer + 4, attribute.valueLength - 4);
                    std::cout << "STUN: Error " << errorCode << " received: " << reason << std::endl;
                }
            }
        }
        else
        {
            std::cout << "STUN: Invalid response type: " << responseType << std::endl;
        }
    }
    return std::make_tuple(false, "", 0);
}

unsigned int STUNClient::createRequestMessage(STUNMessageType type, const std::string& messageBody)
{
    uint16_t requestType = htons(type);
    memcpy((void*)buffer, &requestType, 2);
    uint16_t bodySize = htons((uint16_t)messageBody.size());
    memcpy((buffer + 2), &bodySize, 2);
    uint32_t magicCookie = htonl(MAGIC_COOKIE);
    memcpy((buffer + 4), &magicCookie, 4);
    createTransactionID((char*)(buffer + 8));
    transactionID = std::string(buffer + 8, 12);
    if(messageBody.size() > 0)
    {
        memcpy((buffer + 20), messageBody.data(), messageBody.size());
    }
    return messageBody.size() + STUN_HEADER_SIZE;
}

void STUNClient::createTransactionID(char* position)
{
    std::srand(::time(nullptr));
    uint32_t rnd = std::rand();
    memcpy(position, &rnd, 4);
    rnd = std::rand();
    memcpy(position + 4, &rnd, 4);
    rnd = std::rand();
    memcpy(position + 8, &rnd, 4);
}

const STUNClient::STUNAttribute STUNClient::readAttribute(unsigned int maxSize, unsigned int position) const
{
    if(maxSize < position + 2)
    {
        return {STUN_INVALID_ATTRIBUTE, nullptr, 0};
    }
    const uint16_t attributeType = ntohs(*(reinterpret_cast<const uint16_t*>(buffer + position)));
    const uint16_t attributeContentLength = ntohs(*(reinterpret_cast<const uint16_t*>(buffer + position + 2)));
    return {attributeType, (const char*)(buffer + position + 4), attributeContentLength};
}

const std::tuple<std::string, unsigned short> STUNClient::readMappedAddress(const STUNAttribute& attribute, const uint32_t magicCookie)
{
    const uint16_t addressFamily = ntohs(*((uint16_t*)attribute.valuePointer));
    uint16_t port = ntohs(*((uint16_t*)(attribute.valuePointer + 2)));
    //RFC 5389 section 15.2:
    //"X-Port is computed by taking the mapped port in host byte order,
    //XOR'ing it with the most significant 16 bits of the magic cookie"
    if(attribute.type == STUN_XOR_MAPPED_ADDRESS)
    {
        port = port ^ (magicCookie >> 16);
    }
    if(addressFamily == MAPPED_ADDRESS_IPv4)
    {
        unsigned char* addressPtr = (unsigned char*)attribute.valuePointer + 4;
        const uint32_t xoredAddress = *((uint32_t*)addressPtr) ^ magicCookie;
        std::string address("");
        if(attribute.type == STUN_XOR_MAPPED_ADDRESS)
        {
            //RFC 5389 section 15.2:
            //"If the IP address family is IPv4, X-Address is computed by taking the mapped IP address in host byte order, 
            //XOR'ing it with the magic cookie."
            addressPtr = reinterpret_cast<unsigned char*>(xoredAddress);
        }
        address.append(std::to_string(*addressPtr)).append(".");
        address.append(std::to_string(*(addressPtr+1))).append(".");
        address.append(std::to_string(*(addressPtr+2))).append(".");
        address.append(std::to_string(*(addressPtr+3)));
        return std::make_tuple(address, port);
    }
    else if(addressFamily == MAPPED_ADDRESS_IPv6)
    {
        const unsigned char* addressPtr = (unsigned char*)attribute.valuePointer + 4;
        std::string address("");
        //TODO RFC 5389 section 15.2:
        //"If the IP address family is IPv6, X-Address is computed by taking the mapped IP address
        //in host byte order, XOR'ing it with the concatenation of the magic cookie and the 96-bit transaction ID" 
        //read hexadecimal values from ptr and join with ':'
        uint8_t part = *addressPtr;
        address.append(Utility::toHexString(part));
        for(unsigned int i = 1; i < 16; i++)
        {
            address.append(":");
            part = *(addressPtr + 1);
            address.append(Utility::toHexString(part));
        }
        return std::make_tuple(address, port);
    }
    return std::make_tuple("", 0);
}
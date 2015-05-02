/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include <exception>
#include <new>
#include <string.h>

#include "RTPPackage.h"

/*
 * 
 * RTPHeaderExtension
 * 
 */

uint16_t RTPHeaderExtension::copyToBuffer(char* buffer, size_t maxBufferSize)
{
    //write first 4 bytes of the HeaderExtension
    memcpy(buffer, this, 4);
    int bytesCopied = 4;
    if(length > 0)
    {
        //one extension-line has 4 bytes
        memcpy(buffer+bytesCopied, extensions, length * 4);
        bytesCopied += length * 4;
    }
    return bytesCopied;
}

uint16_t RTPHeaderExtension::readFromBuffer(char* buffer, size_t maxBufferSize)
{
    //read first 4 bytes of the HeaderExtension
    memcpy(this, buffer, 4);
    int bytesRead = 4;
    if(length > 0)
    {
        //one extension-line has 4 bytes
        memcpy(extensions, buffer+bytesRead, length * 4);
        bytesRead += length * 4;
    }
    return bytesRead;
}


/*
 * 
 * RTPHeader
 *  
 */

RTPHeader::RTPHeader()
{
    //version is always 2
    version = 2;
    //disable padding by default
    padding = 0;
    //disable extension by default
    extension = 0;
    //set csrc count to 0
    csrc_count = 0;
    //disable marker-flag by default
    marker = 0;
}

uint16_t RTPHeader::copyToBuffer(char* buffer, size_t maxBufferSize)
{
    int bytesCopied = 0;
    //1. copy standard fields
    //standard fields always take 12 bytes
    memcpy(buffer, this, 12);
    bytesCopied = 12;
    //2. copy csrcs (only those, which are really set)
    if(csrc_count > 0)
    {
        //one csrc has 32 bit -> 4 bytes
        memcpy(buffer+bytesCopied, csrc_list, csrc_count * 4);
        bytesCopied += csrc_count * 4;
    }
    //3. copy extensions
    if(extension != 0 && header_extension != NULL)
    {
        bytesCopied += header_extension->copyToBuffer(buffer+bytesCopied, maxBufferSize - bytesCopied);
    }
    return bytesCopied;
}

uint16_t RTPHeader::readFromBuffer(char* buffer, size_t maxBufferSize)
{
    int bytesRead = 0;
    //1. read standard fields
    //standard fields always take 12 bytes
    memcpy(this, buffer, 12);
    bytesRead = 12;
    //2. read csrcs (only those, which are really set)
    if(csrc_count > 0)
    {
        //one csrc has 32 bit -> 4 bytes
        memcpy(csrc_list, buffer+bytesRead, csrc_count * 4);
        bytesRead += csrc_count * 4;
    }
    //3. read extensions
    if(extension != 0)
    {
        header_extension = new RTPHeaderExtension();
        bytesRead += header_extension->readFromBuffer(buffer + bytesRead, maxBufferSize - bytesRead);
    }
    return bytesRead;
}


/*
 * 
 * RTPPackage
 * 
 */

RTPPackage::RTPPackage()
{
    
}

RTPPackage::RTPPackage(RTPHeader header, size_t packageSize, void* packageData)
{
    this->header = header;
    package = packageData;
    this->packageSize = packageSize;
}

uint16_t RTPPackage::copyToBuffer(char* buffer, size_t maxBufferSize)
{
    int bytesCopied = header.copyToBuffer(buffer, maxBufferSize);
    memcpy(buffer + bytesCopied, package, packageSize);
    bytesCopied += packageSize;
    return bytesCopied;
}

uint16_t RTPPackage::readFromBuffer(char* buffer, size_t maxBufferSize)
{
    int bytesRead = header.readFromBuffer(buffer, maxBufferSize);
    packageSize = maxBufferSize - bytesRead;
    package = malloc(packageSize);
    if(package == NULL)
    {
        return -1;
    }
    memcpy(package, buffer+bytesRead,packageSize);
    return maxBufferSize;
}

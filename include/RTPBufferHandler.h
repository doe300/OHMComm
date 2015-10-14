#ifndef RTPBUFFERHANDLER_H
#define	RTPBUFFERHANDLER_H

#include "RTPPackageHandler.h"
#include "Statistics.h"

//#include <malloc.h>
#include <memory> //for std::unique_ptr<RTPBuffer>


/*!
* This status is returned by the addPackage/readPackage-method to determine whether the operation did succeed
*/
enum class RTPBufferStatus : char
{
    RTP_BUFFER_ALL_OKAY,
    /*!
    * New package was not buffered, because of an overflow in the buffer
    */
    RTP_BUFFER_INPUT_OVERFLOW,
    /*!
    * No package to read, because of an underflow in the buffer
    */
    RTP_BUFFER_OUTPUT_UNDERFLOW,
    /*!
    * The package was thrown away because the sequence number of the received package was lower than the current read sequence number
    * Example: The last read sequence number was 55. If it now receives the sequence number 54 or lower it will be thrown away.
    * The received package is to old, therefore it will not be processed.
    */
    RTP_BUFFER_PACKAGE_TO_OLD,

    RTP_BUFFER_IS_PUFFERING
};

/*!
* Abstract super-type for all classes used as RTPBuffer (Jitter-Buffer)
*/
class RTPBufferHandler
{
public:

    virtual ~RTPBufferHandler()
    {

    }

    /*!
     * Adds a packet to the buffer
     *
     * \param package The package to add
     *
     * \param contentSize The size of the audio data in the package
     */
    virtual RTPBufferStatus addPackage(RTPPackageHandler &package, unsigned int contentSize) = 0;

    /*!
     * Reads a package from the buffer and writes its content into the given parameter
     *
     * \param package The object in which the read data will be written in
     */
    virtual RTPBufferStatus readPackage(RTPPackageHandler &package) = 0;

    /*!
     * Returns the number of currently buffered packages
     */
    virtual unsigned int getSize() const = 0;
};

#endif

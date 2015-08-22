#ifndef RTPBUFFERHANDLER_H
#define	RTPBUFFERHANDLER_H

#include "RTPPackageHandler.h"
#include "Statistics.h"

//#include <malloc.h>
#include <memory> //for std::unique_ptr<RTPBuffer>


/*!
* This status is returned by the addPackage/readPackage-method to determine whether the operation did succeed
*/
typedef uint8_t RTPBufferStatus;
static const RTPBufferStatus RTP_BUFFER_ALL_OKAY = 0;
/*!
* New package was not buffered, because of an overflow in the buffer
*/
static const RTPBufferStatus RTP_BUFFER_INPUT_OVERFLOW = 0x1;
/*!
* No package to read, because of an underflow in the buffer
*/
static const RTPBufferStatus RTP_BUFFER_OUTPUT_UNDERFLOW = 0x2;

/*!
* Abstract super-type for all classes used as RTBBuffer (Jitter-Buffer)
*/
class RTPBufferHandler
{
public:
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
};

#endif
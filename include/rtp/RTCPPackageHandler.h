/*
 * File:   RTCPPackageHandler.h
 * Author: daniel
 *
 * Created on June 16, 2015, 5:47 PM
 */

#ifndef RTCPPACKAGEHANDLER_H
#define	RTCPPACKAGEHANDLER_H

#include <vector>

#include "rtp/RTCPHeader.h"

/*!
 * Utility-class to create/read RTCP packages
 *
 * NOTE: This class is not thread safe!
 *
 * NOTE: All methods in this class modify some of their parameter!
 */
class RTCPPackageHandler
{
public:

    static constexpr uint8_t RTCP_HEADER_SIZE = sizeof(RTCPHeader);
    static constexpr uint8_t RTCP_SENDER_INFO_SIZE = sizeof(SenderInformation);
    static constexpr uint8_t RTCP_RECEPTION_REPORT_SIZE = sizeof(ReceptionReport);

    RTCPPackageHandler();

    virtual ~RTCPPackageHandler();

    /*!
     * Creates a new sender report (SR) package
     *
     * \param header The package-header
     *
     * \param senderInfo The SenderInformation
     *
     * \param reports A (possible empty) list of reception reports
     * 
     * \param offset An optional offset to create an element of a RTCP compound package
     *
     * \return A pointer to the created package
     */
    const void *createSenderReportPackage(RTCPHeader &header, const SenderInformation &senderInfo, const std::vector<ReceptionReport>& reports, const unsigned int offset = 0);

    /*!
     * Creates a new receiver report (RR) package
     *
     * \param header The package-header
     *
     * \param reports A (possible empty) list of reception reports
     * 
     * \param offset An optional offset to create an element of a RTCP compound package
     *
     * \return A pointer to the created package
     */
    const void *createReceiverReportPackage(RTCPHeader &header, const std::vector<ReceptionReport>& reports, const unsigned int offset = 0);

    /*!
     * Creates a new source description (DES) package
     *
     * \param header The header of this package
     *
     * \param descriptions The vector of SourceDescription
     * 
     * \param offset An optional offset to create an element of a RTCP compound package
     *
     * \return A pointer to the created package
     */
    const void *createSourceDescriptionPackage(RTCPHeader &header, const std::vector<SourceDescription>& descriptions, const unsigned int offset = 0);

    /*!
     * Creates a new BYE package
     *
     * \param header The header
     *
     * \param byeMessage The message to send with this package (possible empty)
     * 
     * \param offset An optional offset to create an element of a RTCP compound package
     *
     * \return A pointer to the created package
     */
    const void *createByePackage(RTCPHeader &header, const std::string& byeMessage, const unsigned int offset = 0);

    /*!
     * Creates a new APP package
     *
     * \param header The header
     *
     * \param appData The application specific data
     * 
     * \param offset An optional offset to create an element of a RTCP compound package
     *
     * \return A pointer to the created package
     */
    const void *createApplicationDefinedPackage(RTCPHeader &header, ApplicationDefined &appDefined, const unsigned int offset = 0);

    /*!
     * Reads a sender report (SR) package
     *
     * \param senderReportPackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \param senderInfo The SenderInformation to store the read sender-info into
     *
     * \return a list of read reception-reports, may be empty
     */
    std::vector<ReceptionReport> readSenderReport(const void *senderReportPackage, uint16_t packageLength, RTCPHeader &header, SenderInformation &senderInfo) const;

    /*!
     * Reads a receiver report (RR) package
     *
     * \param receiverReportPackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return a list of read reception-reports, may be empty
     */
    std::vector<ReceptionReport> readReceiverReport(const void *receiverReportPackage, uint16_t packageLength, RTCPHeader &header) const;

    /*!
     * Reads a source description (SDES) package
     *
     * \param sourceDescriptionPackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return the read descriptions
     */
    std::vector<SourceDescription> readSourceDescription(const void *sourceDescriptionPackage, uint16_t packageLength, RTCPHeader &header) const;

    /*!
     * Reads a BYE package
     *
     * \param byePackage The buffer to read the package from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return the bye-message attached to the package, may be empty
     */
    std::string readByeMessage(const void *byePackage, uint16_t packageLength, RTCPHeader &header) const;

    /*!
     * Reads a APP package
     *
     * \param appDefinedPackage The buffer to read from
     *
     * \param packageLength The number of bytes to read
     *
     * \param header The RTCPHeader to store the read header into
     *
     * \return the read ApplicationDefined data
     */
    ApplicationDefined readApplicationDefinedMessage(const void *appDefinedPackage, uint16_t packageLength, RTCPHeader &header) const;

    /*!
     * Reads an RTCP-header and returns whether the package was an RTCP-package
     *
     * \param rtcpPackage The buffer to read from
     *
     * \param packageLength The number of bytes in the package
     *
     * \return Whether the RTCP-header was successfully read
     */
    RTCPHeader readRTCPHeader(const void* rtcpPackage, unsigned int packageLength) const;

    /*!
     * This method tries to determine whether the received buffer holds an RTCP package.
     *
     * NOTE: Though this method can distinguish RTCP-packages from RTP-packages,
     * there may be some type of packages, which are not distinguished and falsely accepted as valid RTCP-package.
     *
     * \param packageBuffer The buffer storing the package-data
     *
     * \param packageLength The length of the package in bytes
     *
     * \return Whether this buffer COULD be holding hold an RTCP package
     */
    static bool isRTCPPackage(const void* packageBuffer, const unsigned int packageLength );

    /*!
     * \param lengthHeaderField The value of the RTCP header-field "length"
     *
     * \return the length of the RTCP-package in bytes
     */
    static unsigned int getRTCPPackageLength(unsigned int lengthHeaderField);
    
    /*!
     * \param rtcpCompoundBuffer The buffer containing the (possible) compound package
     * 
     * \param maxPackageLength The maximum length of the package
     * 
     * \return The number of RTCP packages currently in this buffer
     */
    static unsigned int getRTCPCompoundPackagesCount(const void* rtcpCompoundBuffer, const unsigned int maxPackageLength);

private:
    std::vector<char> rtcpPackageBuffer;

    /*!
     * Calculates the length-field from the given length in bytes
     *
     * Specification: "The length of this RTCP packet in 32-bit words minus one, including the header and any padding"
     */
    static uint8_t calculateLengthField(uint16_t length);
    
    void assertCapacity(unsigned int newCapacity);
    
    friend class RTCPHandler;
};

#endif	/* RTCPPACKAGEHANDLER_H */


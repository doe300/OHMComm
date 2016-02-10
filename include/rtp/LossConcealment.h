/* 
 * File:   LossConcealment.h
 * Author: daniel
 *
 * Created on January 28, 2016, 10:15 AM
 */

#ifndef LOSSCONCEALMENT_H
#define	LOSSCONCEALMENT_H

#include "RTPPackageHandler.h"

/*!
 * Mixin for jitter buffers to improve the loss concealment techniques
 * 
 * For the first N lost packages, the last successfully received package is repeated. 
 * After the threshold is reached (more successive packages are lost) the losses are concealed, currently by playing silence
 */
class LossConcealment
{
protected:

    /*!
     * \param maxPackageRepetitions the maximum times a single package is repeated to conceal subsequent lost packages
     */
    LossConcealment(const uint16_t maxPackageRepetitions = DEFAULT_PACKAGE_REPETITIONS) : maxPackageRepetitions(maxPackageRepetitions), 
            numRepeatedPackages(0), lastReceivedSequenceNumber(0)
    {
        
    }
    
    virtual ~LossConcealment()
    {

    }
    
    /*!
     * Conceals the loss of the package specified by the sequence-number
     */
    inline void concealLoss(RTPPackageHandler &package, const uint16_t sequenceNumber)
    {
        if(lastReceivedSequenceNumber != 0 && ((lastReceivedSequenceNumber + numRepeatedPackages + 1)%UINT16_MAX) == sequenceNumber)
        {
            //we have several subsequent losses
            ++numRepeatedPackages;
        }
        else
        {
            //the previous sequence number was received successfully
            numRepeatedPackages = 1;
            //the previous sequence number was the last one received
            lastReceivedSequenceNumber = sequenceNumber == 0 ? UINT16_MAX : sequenceNumber - 1;
        }
        //we are under the repetition threshold, so repeat last successful package, if possible
        if(numRepeatedPackages <= maxPackageRepetitions && repeatLastPackage(package, lastReceivedSequenceNumber))
        {
            //XXX fade out volume on multiple losses (relative to maximum repetitions)???
            // see http://flylib.com/books/en/4.245.1.63/1/
            return;
        }
        //fall back to default concealment
        createConcealmentPackage(package);
    }
    
    /*!
     * Copies the last received package, if it is still in the buffer
     * 
     * \param package the package-handler to copy into
     * \param packageSequenceNumber the sequence-number of the package to copy
     * 
     * \return if the last received package was successfully repeated
     */
    virtual bool repeatLastPackage(RTPPackageHandler &package, const uint16_t packageSequenceNumber) = 0;
    
    /*!
     * Creates a concealment package
     */
    inline void createConcealmentPackage(RTPPackageHandler &package)
    {
        //TODO switch to comfort noise
        package.createSilencePackage();
    }
    
private:
    static constexpr unsigned int DEFAULT_PACKAGE_REPETITIONS{2};
    const uint16_t maxPackageRepetitions;
    uint16_t numRepeatedPackages;
    uint16_t lastReceivedSequenceNumber;
};

#endif	/* LOSSCONCEALMENT_H */


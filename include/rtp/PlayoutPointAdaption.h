/* 
 * File:   PlayoutPointAdaption.h
 * Author: daniel
 *
 * Created on January 27, 2016, 1:33 PM
 */

#ifndef PLAYOUTPOINTADAPTION_H
#define	PLAYOUTPOINTADAPTION_H

/*!
 * Mixin to support dynamic playout point adaption for jitter-buffers
 */
class PlayoutPointAdaption
{
protected:
    
    /*!
     * \param adaptionCycle The number of package receptions before adapting the playout delay
     * \param startPackagesDelay the initial playout delay
     */
    PlayoutPointAdaption(const uint16_t adaptionCycle, const uint16_t startPackagesDelay = 0) : adaptionCycle(adaptionCycle),
            numDelayPackages(startPackagesDelay), numLateLosses(0), numWrites(0)
    {
        
    }
    
    virtual ~PlayoutPointAdaption()
    {
    }

    /*!
     * \return whether the buffer to generate the artificial playout delay is filled and the next package can be played out
     */
    inline bool isAdaptionBufferFilled() const
    {
        return getSize() > numDelayPackages;
    }
    
    /*!
     * Call this method to update the playout point adaption.
     * 
     * NOTE: this method must be called on every package reception
     * 
     * \param isLateLoss whether the received package was a late loss
     */
    inline void packageReceived(const bool isLateLoss)
    {
        //increase number of packages received since last update
        ++numWrites;
        if(isLateLoss)
            //increase number of late losses since last update
            ++numDelayPackages;
        
        //adapt playout point
        if(numWrites == adaptionCycle)
        {
            const double sharedLoss = ((double)numLateLosses) / numWrites;
            //if late loss less than a certain threshold, decrease delay
            if(sharedLoss < LATE_LOSS_DECREASE_THRESHOLD)
            {
                if(numDelayPackages > 0)
                    --numDelayPackages;
            }
            //if late loss more than a certain threshold, increase delay
            else if(sharedLoss > LATE_LOSS_INCREASE_THRESHOLD)
            {
                ++numDelayPackages;
            }
            //reset values
            numWrites = 0;
            numLateLosses = 0;
        }
    }
    
    /*!
     * \return the number of packages in this buffer
     */
    virtual unsigned int getSize() const = 0;
    
private:
    //XXX better thresholds?
    constexpr static double LATE_LOSS_INCREASE_THRESHOLD = 0.3;
    constexpr static double LATE_LOSS_DECREASE_THRESHOLD = 0.05;
    //interval for adapting the playout point, in number of package receptions
    const uint16_t adaptionCycle;
    //number of packages to be held back to generate the required delay
    uint16_t numDelayPackages;
    //number of late losses since the last adaption
    uint16_t numLateLosses;
    //number of packages received since the last adaption
    uint16_t numWrites;
};

#endif	/* PLAYOUTPOINTADAPTION_H */


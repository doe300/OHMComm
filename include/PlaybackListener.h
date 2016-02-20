/* 
 * File:   PlaybackListener.h
 * Author: daniel
 *
 * Created on December 16, 2015, 4:15 PM
 */

#ifndef PLAYBACKLISTENER_H
#define	PLAYBACKLISTENER_H

#include <vector>
#include <memory>
#include <functional>

/*!
 * Defined to prevent circular includes when OHMComm uses this header to maintain a structure of listeners
 */
class PlaybackObservee;

/*!
 * Implementation of the observer/listener design-pattern to be notified on playback changes.
 * 
 */
class PlaybackListener
{
public:
    
    /*!
     * This method is invoked on registering this listener
     * 
     * \param ohmComm the observee to be bound to
     */
    virtual void onRegister(PlaybackObservee* ohmComm)
    {
        //do nothing
    };
    
    /*!
     * This method is invoked on playback start
     */
    virtual void onPlaybackStart()
    {
        //do nothing
    };
    
    /*!
     * This method is invoked after playback ended
     */
    virtual void onPlaybackStop()
    {
        //do nothing
    };
};

/*!
 * Manages the playback-listeners and provides method for notifying them
 */
class PlaybackObservee
{
public:
    
    /*!
     * \param listener The listener to register
     */
    void registerPlaybackListener(const std::shared_ptr<PlaybackListener> listener)
    {
        if(listener != nullptr)
        {
            listeners.push_back(listener);
            listener->onRegister(this);
        }
    }
    
    /*!
     * \return a function, which will stop the playback on invocation
     */
    virtual std::function<void ()> createStopCallback() = 0;
    
protected:
    
    PlaybackObservee() : listeners()
    {
        
    }
    
    virtual ~PlaybackObservee()
    {
        
    }
    
    /*!
     * Notifies all listeners about starting of playback
     */
    void notifyPlaybackStart()
    {
        for(const std::shared_ptr<PlaybackListener>& l : listeners)
        {
            l->onPlaybackStart();
        }
    }
    
    /*!
     * Notifies all listeners, that playback has stopped
     */
    void notifyPlaybackStop()
    {
        for(const std::shared_ptr<PlaybackListener>& l : listeners)
        {
            l->onPlaybackStop();
        }
    }
    
private:
    std::vector<std::shared_ptr<PlaybackListener>> listeners;
};

#endif	/* PLAYBACKLISTENER_H */


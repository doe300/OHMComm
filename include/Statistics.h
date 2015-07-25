/* 
 * File:   Statistics.h
 * Author: daniel
 *
 * Created on June 30, 2015, 5:06 PM
 */

#ifndef STATISTICS_H
#define	STATISTICS_H

#include <iostream>
#include <string>
#include "AudioProcessorStatistic.h"
#include <vector>
/*!
 * Class to collect and print statistical information
 */
class Statistics
{
public:
    static const int COUNTER_PACKAGES_SENT;
    static const int COUNTER_PACKAGES_RECEIVED;
    static const int COUNTER_PACKAGES_LOST;
    static const int COUNTER_FRAMES_SENT;
    static const int COUNTER_FRAMES_RECORDED;
    static const int COUNTER_FRAMES_RECEIVED;
    static const int COUNTER_FRAMES_OUTPUT;
    static const int COUNTER_HEADER_BYTES_SENT;
    static const int COUNTER_HEADER_BYTES_RECEIVED;
    static const int COUNTER_PAYLOAD_BYTES_SENT;
    static const int COUNTER_PAYLOAD_BYTES_RECORDED;
    static const int COUNTER_PAYLOAD_BYTES_RECEIVED;
    static const int COUNTER_PAYLOAD_BYTES_OUTPUT;
    static const int TOTAL_ELAPSED_MILLISECONDS;
    static const int RTP_BUFFER_MAXIMUM_USAGE;
    static const int RTP_BUFFER_LIMIT;
    
    /*!
     * Increments the given counter by the value provided
     * 
     * \param counterIndex The key of the counter to increment
     * 
     * \param byValue The value to increment by, defaults to 1
     */
    static void incrementCounter(int counterIndex, long byValue = 1);
    
    /*!
     * Sets the given counter to the value provided
     * 
     * \param counterIndex The key to the counter
     * 
     * \param newValue The value to set
     */
    static void setCounter(int counterIndex, long newValue);
    
    /*!
     * Sets the given counter to the maximum of its old value and newValue
     * 
     * \param counterIndex The key to the counter
     * 
     * \param newValue The new value to compare and possibly set
     */
    static void maxCounter(int counterIndex, long newValue);
    
    /*!
     * Prints some general statistical information
     */
    static void printStatistics();

	/*!
	 * Adds a processor to statistics array in order to record its statistic
	 *
	 * \param name The name of the AudioProcessor
	 */
	static void addProcessor(std::string name);

	/*!
	 * Removes a processor from statistics array
	 *
	 * \param name The name of the AudioProcessor
	 */
	static void removeProcessor(std::string name);

	/*!
	 * Removes all processors from the statistics array
	 */
	static void removeAllProcessors();

	/*!
	 * Get the specific AudioProcessorStatistic
	 */
	static auto getAudioProcessorStatistic(std::string name) -> AudioProcessorStatistic*;

	/*!
	 * Prints statistics from all added AudioProcessors
	 */
	static void printAudioProcessorStatistic();

	/*!
	 * Starts the timer for an AudioProcessorStatistic-Object (Input)
	 *
	 * \param name The name of the AudioProcessor
	 */
	static void TimerStartInputProcessing(std::string name);

	/*!
	 * Stops the timer for an AudioProcessorStatistic-Object (Input)
	 *
	 * \param name The name of the AudioProcessor
	 */
	static void TimerStopInputProcessing(std::string name);

	/*!
	 * Starts the timer for an AudioProcessorStatistic-Object (Output)
	 *
	 * \param name The name of the AudioProcessor
	 */
	static void TimerStartOutputProcessing(std::string name);

	/*!
	 * Stops the timer for an AudioProcessorStatistic-Object (Output)
	 *
	 * \param name The name of the AudioProcessor
	 */
	static void TimerStopOutputProcessing(std::string name);
private:

    static long counters[20];
    
    static double prettifyPercentage(double percentage);
    
    static std::string prettifyByteSize(double byteSize);

	static std::vector<AudioProcessorStatistic*> Statistics::audioProcessorStatistics;
};



#endif	/* STATISTICS_H */


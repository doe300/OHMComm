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
#include <vector>

#include "ProfilingAudioProcessor.h"
/*!
 * Class to collect and print statistical information
 */
class Statistics
{
public:
    static constexpr int COUNTER_PACKAGES_SENT{0};
    static constexpr int COUNTER_PACKAGES_RECEIVED{1};
    static constexpr int COUNTER_PACKAGES_LOST{2};
    static constexpr int COUNTER_FRAMES_SENT{3};
    static constexpr int COUNTER_FRAMES_RECORDED{4};
    static constexpr int COUNTER_FRAMES_RECEIVED{5};
    static constexpr int COUNTER_FRAMES_OUTPUT{6};
    static constexpr int COUNTER_HEADER_BYTES_SENT{7};
    static constexpr int COUNTER_HEADER_BYTES_RECEIVED{8};
    static constexpr int COUNTER_PAYLOAD_BYTES_SENT{9};
    static constexpr int COUNTER_PAYLOAD_BYTES_RECORDED{10};
    static constexpr int COUNTER_PAYLOAD_BYTES_RECEIVED{11};
    static constexpr int COUNTER_PAYLOAD_BYTES_OUTPUT{12};
    static constexpr int TOTAL_ELAPSED_MILLISECONDS{13};
    static constexpr int RTP_BUFFER_MAXIMUM_USAGE{14};
    static constexpr int RTP_BUFFER_LIMIT{15};

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
     * Prints some general statistical information to stdout
     */
    static void printStatistics();

    /*!
     * Prints the statistical information to the file specified by the given name
     */
    static void printStatisticsToFile(std::string fileName);

    /*!
     * Adds a processor to statistics array in order to record its statistic
     *
     * \param profiler The profiling audio-processor
     */
    static void addProfiler(ProfilingAudioProcessor* profiler);

    /*!
     * Removes a processor from statistics array
     *
     * \param profiler The profiling audio-processor
     */
    static void removeProfiler(ProfilingAudioProcessor* profiler);

    /*!
     * Removes all processors from the statistics array
     */
    static void removeAllProfilers();

private:

    static long counters[20];

    static double prettifyPercentage(double percentage);

    static std::string prettifyByteSize(double byteSize);

    static std::vector<ProfilingAudioProcessor*> audioProcessorStatistics;

    /*!
     * Internal helper-method to print statistics to given output-stream
     */
    static void printStatistics(std::ostream& outputStream);
    /*!
     * Internal helper-method to print audio-processor profiling results to given output-stream
     */
    static void printAudioProcessorStatistic(std::ostream& outputStream);
};



#endif	/* STATISTICS_H */


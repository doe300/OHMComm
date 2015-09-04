/*
 * File:   Statistics.cpp
 * Author: daniel
 *
 * Created on June 30, 2015, 5:06 PM
 */

#include "Statistics.h"
#include <fstream>

long Statistics::counters[] = {0};
std::vector<ProfilingAudioProcessor*> Statistics::audioProcessorStatistics;

void Statistics::incrementCounter(int counterIndex, long byValue)
{
    counters[counterIndex] += byValue;
}

void Statistics::setCounter(int counterIndex, long newValue)
{
    counters[counterIndex] = newValue;
}

void Statistics::maxCounter(int counterIndex, long newValue)
{
    if(newValue > counters[counterIndex])
    {
        counters[counterIndex] = newValue;
    }
}

void Statistics::printStatistics()
{
    printStatistics(std::cout);
}

void Statistics::printStatisticsToFile(const std::string fileName)
{
    std::ofstream fileStream(fileName.c_str(), std::ios_base::out|std::ios_base::trunc);
    if(fileStream.is_open())
    {
        printStatistics(fileStream);
    }
    else
    {
        std::cerr << "Error while opening log-file!" << std::endl;
    }
    fileStream.close();
    if(fileStream.fail())
    {
        std::cerr << "Error while writing statistics!" << std::endl;
    }
}

double Statistics::prettifyPercentage(double percentage)
{
    int tmp = percentage * 10000;
    return tmp / 100.0;
}

std::string Statistics::prettifyByteSize(double byteSize)
{
    std::string unit;
    double dimensionValue;
    if(byteSize > 1024 * 1024 * 1024)
    {
        unit = " GB";
        dimensionValue = byteSize / (1024.0 * 1024.0 * 1024.0);
    }
    else if(byteSize > 1024 * 1024)
    {
        unit = " MB";
        dimensionValue = byteSize / (1024.0 * 1024.0);
    }
    else if(byteSize > 1024)
    {
        unit = " KB";
        dimensionValue = byteSize / 1024.0;
    }
    else
    {
        unit = " B";
        dimensionValue = byteSize;
    }
    char buf[8];
    int numChars = std::sprintf(buf, "%.2f", dimensionValue);
    return std::string(buf, numChars) + unit;
}

void Statistics::addProfiler(ProfilingAudioProcessor* profiler)
{
    Statistics::audioProcessorStatistics.push_back(profiler);
}


void Statistics::removeProfiler(ProfilingAudioProcessor* profiler)
{
    for (unsigned int i = 0; i < Statistics::audioProcessorStatistics.size(); i++)
    {
        if (Statistics::audioProcessorStatistics.at(i) ==profiler)
        {
            Statistics::audioProcessorStatistics.erase(Statistics::audioProcessorStatistics.begin() + i);
        }
    }
}


void Statistics::removeAllProfilers()
{
    Statistics::audioProcessorStatistics.clear();
}

void Statistics::printStatistics(std::ostream& outputStream)
{
    double seconds = counters[TOTAL_ELAPSED_MILLISECONDS] / 1000.0;
    if(seconds == 0)
    {
        outputStream << "Couldn't print statistics" << std::endl;
        return;
    }
    outputStream << std::endl;
    outputStream << "Ran " << counters[TOTAL_ELAPSED_MILLISECONDS] << " ms (" << seconds << " s)" << std::endl;
    //Audio statistics
    outputStream << std::endl;
    outputStream << "+++ Audio statistics +++" << std::endl;
    outputStream << "Recorded " << counters[COUNTER_PAYLOAD_BYTES_RECORDED] << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]) << ") of audio-data ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]/seconds) << "/s)" << std::endl;
    outputStream << "Recorded " << counters[COUNTER_FRAMES_RECORDED] << " audio-frames ("
            << (counters[COUNTER_FRAMES_RECORDED]/seconds) << " fps)" << std::endl;
    outputStream << "Played " << counters[COUNTER_PAYLOAD_BYTES_OUTPUT] << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]) << ") of audio-data ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]/seconds) << "/s)" << std::endl;
    outputStream << "Played " << counters[COUNTER_FRAMES_OUTPUT] << " audio-frames ("
            << (counters[COUNTER_FRAMES_OUTPUT]/seconds) << " fps)" << std::endl;
    //Network statistics
    outputStream << std::endl;
    outputStream << "+++ Network statistics +++" << std::endl;
    outputStream << "Sent " << counters[COUNTER_PACKAGES_SENT] << " packages ("
            << (counters[COUNTER_PACKAGES_SENT]/seconds) << " per second)" << std::endl;
    outputStream << "Sent " << (counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT]) << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT]) << ") in total ("
            << prettifyByteSize((counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT])/seconds) << "/s)"
            << std::endl;
    outputStream << "Sent " << counters[COUNTER_PAYLOAD_BYTES_SENT] << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]) << ") of audio-data ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]/seconds) << "/s)" << std::endl;
    outputStream << "Sent " << counters[COUNTER_HEADER_BYTES_SENT] << " bytes ("
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_SENT]) << ") of RTP-header ("
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_SENT]/seconds) << "/s)" << std::endl;
    outputStream << "Sent " << counters[COUNTER_HEADER_BYTES_SENT] << " of "
            << counters[COUNTER_PAYLOAD_BYTES_SENT] << " bytes as overhead ("
            << prettifyPercentage(counters[COUNTER_HEADER_BYTES_SENT] / (double) counters[COUNTER_PAYLOAD_BYTES_SENT])
            << "%)" << std::endl;
    outputStream << "Received " << counters[COUNTER_PACKAGES_RECEIVED] << " packages ("
            << (counters[COUNTER_PACKAGES_RECEIVED]/seconds) << " per second)" << std::endl;
    outputStream << "Received " << (counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED]) << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED]) << ") in total ("
            << prettifyByteSize((counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED])/seconds) << "/s)"
            << std::endl;
    outputStream << "Received " << counters[COUNTER_PAYLOAD_BYTES_RECEIVED] << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) << ") of audio-data ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]/seconds) << "/s)" << std::endl;
    outputStream << "Received " << counters[COUNTER_HEADER_BYTES_RECEIVED] << " bytes ("
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_RECEIVED]) << ") of RTP-header ("
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_RECEIVED]/seconds) << "/s)" << std::endl;
    outputStream << "Received " << counters[COUNTER_HEADER_BYTES_RECEIVED] << " of "
            << counters[COUNTER_PAYLOAD_BYTES_RECEIVED] << " bytes as overhead ("
            << prettifyPercentage(counters[COUNTER_HEADER_BYTES_RECEIVED] / (double) counters[COUNTER_PAYLOAD_BYTES_RECEIVED])
            << "%)" << std::endl;
    outputStream << "Lost " << counters[COUNTER_PACKAGES_LOST] << " RTP-packages ("
            << (counters[COUNTER_PACKAGES_LOST]/seconds) << " packages per second)" << std::endl;
    //Buffer statistics
    outputStream << std::endl;
    outputStream << "+++ Buffer statistics +++" << std::endl;
    outputStream << "Maximum buffer usage was " << counters[RTP_BUFFER_MAXIMUM_USAGE] << " of "
            << counters[RTP_BUFFER_LIMIT] << " packages ("
            << prettifyPercentage(counters[RTP_BUFFER_MAXIMUM_USAGE]/(double)counters[RTP_BUFFER_LIMIT]) << "%)"
            << std::endl;
    //Compression statistics
    outputStream << std::endl;
    outputStream << "+++ Compression statistics +++" << std::endl;
    outputStream << "Compressed " << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]) << " of audio-data into "
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]) << " ("
            << prettifyPercentage(1 - (counters[COUNTER_PAYLOAD_BYTES_SENT] / (double) counters[COUNTER_PAYLOAD_BYTES_RECORDED]))
            << "% compression)" << std::endl;
    outputStream << "Decompressed " << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]) << " of audio-data out of "
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) << " ("
            << prettifyPercentage(1.0 - (counters[COUNTER_PAYLOAD_BYTES_RECEIVED] / (double) counters[COUNTER_PAYLOAD_BYTES_OUTPUT]))
            << "% decompression)" << std::endl;

    // AudioProcessor statistics
    Statistics::printAudioProcessorStatistic(outputStream);
}

void Statistics::printAudioProcessorStatistic(std::ostream& outputStream)
{
    if(Statistics::audioProcessorStatistics.empty())
    {
        return;
    }
    // AudioProcessor statistics
    outputStream << std::endl;
    outputStream << "+++ AudioProcessor statistics +++" << std::endl;
    for(ProfilingAudioProcessor* profiler : Statistics::audioProcessorStatistics)
    {
        double inputAverage = profiler->getTotalInputTime() / (double)profiler->getTotalCount();
        double outputAverage = profiler->getTotalOutputTime() / (double)profiler->getTotalCount();
        outputStream << std::endl << profiler->getName() << std::endl;
        outputStream << "\tProcessing audio-input took " << profiler->getTotalInputTime()
                << " microseconds in total (" << inputAverage << " microseconds per call)" << std::endl;
        outputStream << "\tProcessing audio-output took  " << profiler->getTotalOutputTime()
                << " microseconds in total (" << outputAverage << " microseconds per call)" << std::endl;
    }
}

/* 
 * File:   Statistics.cpp
 * Author: daniel
 * 
 * Created on June 30, 2015, 5:06 PM
 */

#include "Statistics.h"

const int Statistics::COUNTER_PACKAGES_SENT = 0;
const int Statistics::COUNTER_PACKAGES_RECEIVED = 1;
const int Statistics::COUNTER_PACKAGES_LOST = 2;
const int Statistics::COUNTER_FRAMES_SENT = 3;
const int Statistics::COUNTER_FRAMES_RECORDED = 4;
const int Statistics::COUNTER_FRAMES_RECEIVED = 5;
const int Statistics::COUNTER_FRAMES_OUTPUT = 6;
const int Statistics::COUNTER_HEADER_BYTES_SENT = 7;
const int Statistics::COUNTER_HEADER_BYTES_RECEIVED = 8;
const int Statistics::COUNTER_PAYLOAD_BYTES_SENT = 9;
const int Statistics::COUNTER_PAYLOAD_BYTES_RECORDED = 10;
const int Statistics::COUNTER_PAYLOAD_BYTES_RECEIVED = 11;
const int Statistics::COUNTER_PAYLOAD_BYTES_OUTPUT = 12;
const int Statistics::TOTAL_ELAPSED_MILLISECONDS = 13;
const int Statistics::RTP_BUFFER_MAXIMUM_USAGE = 14;
const int Statistics::RTP_BUFFER_LIMIT = 15;

long Statistics::counters[] = {0};
std::vector<AudioProcessorStatistic*> Statistics::audioProcessorStatistics;

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
    double seconds = counters[TOTAL_ELAPSED_MILLISECONDS] / 1000.0;
    if(seconds == 0)
    {
        std::cout << "Couldn't print statistics" << std::endl;
        return;
    }
    std::cout << std::endl;
    std::cout << "Ran " << counters[TOTAL_ELAPSED_MILLISECONDS] << " ms (" << seconds << " s)" << std::endl;
    //Audio statistics
    std::cout << std::endl;
    std::cout << "+++ Audio statistics +++" << std::endl;
    std::cout << "Recorded " << counters[COUNTER_PAYLOAD_BYTES_RECORDED] << " bytes (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]) << ") of audio-data (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]/seconds) << "/s)" << std::endl;
    std::cout << "Recorded " << counters[COUNTER_FRAMES_RECORDED] << " audio-frames (" 
            << (counters[COUNTER_FRAMES_RECORDED]/seconds) << " fps)" << std::endl;
    std::cout << "Played " << counters[COUNTER_PAYLOAD_BYTES_OUTPUT] << " bytes (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]) << ") of audio-data (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]/seconds) << "/s)" << std::endl;
    std::cout << "Played " << counters[COUNTER_FRAMES_OUTPUT] << " audio-frames (" 
            << (counters[COUNTER_FRAMES_OUTPUT]/seconds) << " fps)" << std::endl;
    //Network statistics
    std::cout << std::endl;
    std::cout << "+++ Network statistics +++" << std::endl;
    std::cout << "Sent " << counters[COUNTER_PACKAGES_SENT] << " packages (" 
            << (counters[COUNTER_PACKAGES_SENT]/seconds) << " per second)" << std::endl;
    std::cout << "Sent " << (counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT]) << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT]) << ") in total ("
            << prettifyByteSize((counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT])/seconds) << "/s)"
            << std::endl;
    std::cout << "Sent " << counters[COUNTER_PAYLOAD_BYTES_SENT] << " bytes (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]) << ") of audio-data (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]/seconds) << "/s)" << std::endl;
    std::cout << "Sent " << counters[COUNTER_HEADER_BYTES_SENT] << " bytes (" 
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_SENT]) << ") of RTP-header (" 
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_SENT]/seconds) << "/s)" << std::endl;
    std::cout << "Sent " << counters[COUNTER_HEADER_BYTES_SENT] << " of " 
            << counters[COUNTER_PAYLOAD_BYTES_SENT] << " bytes as overhead ("
            << prettifyPercentage(counters[COUNTER_HEADER_BYTES_SENT] / (double) counters[COUNTER_PAYLOAD_BYTES_SENT]) 
            << "%)" << std::endl;
    std::cout << "Received " << counters[COUNTER_PACKAGES_RECEIVED] << " packages (" 
            << (counters[COUNTER_PACKAGES_RECEIVED]/seconds) << " per second)" << std::endl;
    std::cout << "Received " << (counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED]) << " bytes ("
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED]) << ") in total ("
            << prettifyByteSize((counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED])/seconds) << "/s)"
            << std::endl;
    std::cout << "Received " << counters[COUNTER_PAYLOAD_BYTES_RECEIVED] << " bytes (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) << ") of audio-data (" 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]/seconds) << "/s)" << std::endl;
    std::cout << "Received " << counters[COUNTER_HEADER_BYTES_RECEIVED] << " bytes (" 
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_RECEIVED]) << ") of RTP-header (" 
            << prettifyByteSize(counters[COUNTER_HEADER_BYTES_RECEIVED]/seconds) << "/s)" << std::endl;
    std::cout << "Received " << counters[COUNTER_HEADER_BYTES_RECEIVED] << " of " 
            << counters[COUNTER_PAYLOAD_BYTES_RECEIVED] << " bytes as overhead ("
            << prettifyPercentage(counters[COUNTER_HEADER_BYTES_RECEIVED] / (double) counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) 
            << "%)" << std::endl;
    std::cout << "Lost " << counters[COUNTER_PACKAGES_LOST] << " RTP-packages (" 
            << (counters[COUNTER_PACKAGES_LOST]/seconds) << " packages per second)" << std::endl;
    //Buffer statistics
    std::cout << std::endl;
    std::cout << "+++ Buffer statistics +++" << std::endl;
    std::cout << "Maximum buffer usage was " << counters[RTP_BUFFER_MAXIMUM_USAGE] << " of " 
            << counters[RTP_BUFFER_LIMIT] << " packages ("
            << prettifyPercentage(counters[RTP_BUFFER_MAXIMUM_USAGE]/(double)counters[RTP_BUFFER_LIMIT]) << "%)" 
            << std::endl;
    //Compression statistics
    std::cout << std::endl;
    std::cout << "+++ Compression statistics +++" << std::endl;
    std::cout << "Compressed " << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]) << " of audio-data into " 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]) << " (" 
            << prettifyPercentage(1 - (counters[COUNTER_PAYLOAD_BYTES_SENT] / (double) counters[COUNTER_PAYLOAD_BYTES_RECORDED])) 
            << "% compression)" << std::endl;
    std::cout << "Decompressed " << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]) << " of audio-data out of " 
            << prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) << " ("
            << prettifyPercentage((counters[COUNTER_PAYLOAD_BYTES_OUTPUT] / (double) counters[COUNTER_PAYLOAD_BYTES_RECEIVED])) 
            << "% decompression)" << std::endl;

	// AudioProcessor statistics
	Statistics::printAudioProcessorStatistic();
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

void Statistics::addProcessor(std::string name)
{
	Statistics::audioProcessorStatistics.push_back(new AudioProcessorStatistic(name));
}


void Statistics::removeProcessor(std::string name)
{
	for (auto i = 0; i < Statistics::audioProcessorStatistics.size(); i++)
	{
		if (Statistics::audioProcessorStatistics.at(i)->getProcessorName() == name)
			Statistics::audioProcessorStatistics.erase(Statistics::audioProcessorStatistics.begin() + i);
	}
}


void Statistics::removeAllProcessors()
{
	Statistics::audioProcessorStatistics.clear();
}

void Statistics::printAudioProcessorStatistic()
{
	
	// AudioProcessor statistics
	std::cout << std::endl;
	std::cout << "+++ AudioProcessor statistics +++" << std::endl;
	for (auto i = 0; i < Statistics::audioProcessorStatistics.size(); i++)
	{
		std::cout << std::endl << audioProcessorStatistics.at(i)->getProcessorName() << std::endl;
		std::cout << "InputStatistics" << audioProcessorStatistics.at(i)->getAverageInputTime() << std::endl;
		std::cout << "OutputStatistics" << audioProcessorStatistics.at(i)->getAverageOuputTime() << std::endl;
	}
}

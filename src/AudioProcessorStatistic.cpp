#include "AudioProcessorStatistic.h"


void AudioProcessorStatistic::addTimeInputProcessing(int ms)
{ 
	counts++; 
	inputProcessingTime += ms; 
};

void AudioProcessorStatistic::addTimeOutputProcessing(int ms) 
{ 
	counts++; 
	outputProcessingTime += ms; 
}

double AudioProcessorStatistic::getAverageOuputTime()
{ 
	return (double)(outputProcessingTime) / counts;
}

double AudioProcessorStatistic::getAverageInputTime()
{
	return (double)(inputProcessingTime) / counts;
}

std::string AudioProcessorStatistic::getProcessorName() 
{ 
	return processorName; 
}

void AudioProcessorStatistic::reset() 
{ 
	outputProcessingTime = 0; 
	inputProcessingTime = 0; 
	counts = 0; 
}

void Time::start()
{
	timeStart = std::chrono::high_resolution_clock::now();
}

void Time::stop()
{
	timeStop = std::chrono::high_resolution_clock::now();
}

int Time::getDifferenceInMs()
{
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(timeStop - timeStart).count();
	return static_cast<int>(microseconds);
}
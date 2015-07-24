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

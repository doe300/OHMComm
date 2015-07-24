#ifndef AUDIOPROCESSORSTATISTIC
#define	AUDIOPROCESSORSTATISTIC

#include <iostream>
#include <string>

class AudioProcessorStatistic
{
public:
	AudioProcessorStatistic(std::string name) : processorName(name), outputProcessingTime(0), inputProcessingTime(0), counts(0) {};
	void addTimeInputProcessing(int ms);
	void addTimeOutputProcessing(int ms);
	double getAverageInputTime();
	double getAverageOuputTime();
	std::string getProcessorName();
	void reset();
private:
	std::string processorName;
	unsigned int outputProcessingTime;
	unsigned int inputProcessingTime;
	unsigned int counts;
};


#endif
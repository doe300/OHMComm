#ifndef PROCESSORFILE_H

#define	PROCESSORFILE_H
#include <string>

class ProcessorFile
{
public:
	FileProcessor(std::string filename, bool writeMode=false, bool overwriteMode=false);
	~FileProcessor();
	int start();
	int start(AudioConfiguration &audioConfiguration);
	int setConfiguration(AudioConfiguration &audioConfiguration);
	int suspend(); // Unterschied zu stop?
	int stop(); // Unterschied zu suspend?
	int reset(); // stoppen, audioConfiguration wieder zurücksetzen

private:
	void openFile(std::string filename);
	void closeFile();
	void overwriteFile(void &data, unsigned int lengthOfData);
	void appendToFile(void &data, unsigned int lengthOfData);
	void createFile();

	// Only one of these variables can be true at once
	bool writeMode;
	bool readMode;

	// Only one of these variables can be true at once
	bool appendMode;
	bool overwriteMode;

	std::string filename;

	// the fileHandler-Object
	// output folder will be the build folder
	std::fstream file;
}
#endif
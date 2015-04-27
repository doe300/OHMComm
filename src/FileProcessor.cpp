#include "FileProcessor.h"

void FileProcessor::openFile(bool readOnly, bool overwrite)
{
	if (file.is_open() == true)
		return;

	if (readOnly == false) // writeMode
	{
		if (overwrite) // overwrite
			file.open(fileName, std::fstream::out | std::fstream::binary | std::fstream::trunc);
		else // append
			file.open(fileName, std::fstream::out | std::fstream::binary | std::fstream::app);
	}
	else // readOnlyMode
	{
		file.open(fileName, std::fstream::in | std::fstream::binary);
	}
}

void FileProcessor::closeFile()
{
	if (file.is_open())
		file.close();
}

void FileProcessor::appendToFile(void *input)
{
	file << input;
}

FileProcessor::FileProcessor(std::string fileName, bool readOnly, bool overwriteFile)
{
	this->fileName = fileName;
	this->openFile(readOnly, overwriteFile);
}

FileProcessor::~FileProcessor()
{
	closeFile();
}

void FileProcessor::configure()
{

}

int FileProcessor::process(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	unsigned int* buffer = (unsigned int *)(inputBuffer);

	int sizeOfAFrameInBytes = 4; // This needs to be set dynamically

	for (size_t i = 0; i < nBufferFrames; i++)
	{
		file.write((char *)buffer, sizeOfAFrameInBytes);
		buffer++;
	}
	return 0;
}
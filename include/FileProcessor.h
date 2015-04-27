/*
* File:   FileProcessor.h
* Author: Kamal
*
* Created on April 27, 2015, 6:19 PM
*/

#ifndef FILEPROCESSOR_H
#define	FILEPROCESSOR_H

#include <string>
#include <iostream>
#include <fstream>

#include "AudioProcessor.h"

class FileProcessor : public AudioProcessor
{
public:
	FileProcessor(std::string fileName, bool readOnly = true, bool overwriteFile = false);
	~FileProcessor();

	void startRecording();
	void stopRecording();
	void pauseRecording();
	void resumeRecording();

	void startPlayback();
	void stopPlayback();
	void pausePlayback();
	void resumePlayback();

	void openFile(bool readOnly = true, bool overwrite = false);
	void closeFile();
	void appendToFile(void* input);

	// AudioProcessor - Processchain
	int process(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData);
	void configure();
protected:
	std::string fileName;

	// the fileHandler-Object. file will be created in build folder
	std::fstream file;
	

	// RTAudio related:
	void openStream(RtAudio::StreamParameters parameters, unsigned int sampleRate, unsigned int bufferFrames, bool recordMode, RtAudio::StreamParameters output);
	void closeStream();
	int recordingCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData);
	int playingCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData);

	// Variables:
	RtAudio rtaudio;
	AudioConfiguration audioConfig;
};

#endif	/* FILEPROCESSOR_H */
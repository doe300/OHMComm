/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include "RTPPackage.h"

RTPPackage::RTPPackage(unsigned int dataSize, unsigned int rtp_header_size, PayloadType payloadType)
{
	this->dataSize = dataSize;
	this->rtp_header_size = rtp_header_size;
	newRTPPackage_Buffer = new char[dataSize + rtp_header_size];
	rtpPackageRecv_Buffer = new char[dataSize + rtp_header_size];
	readAudioDataFromRTPPackage_Buffer = new char[dataSize];
	readRTPHeaderFromRTPPackage_Buffer = new char[rtp_header_size];


	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 tmp(seed1);
	this->randomGenerator = tmp;

	
	sequenceNr = getRandomNumber();
	timestamp = getTimestamp();
	ssrc = getAudioSourceId();
	this->payloadType = payloadType;
}

auto RTPPackage::getNewRTPPackage(void* data, unsigned int timestamp) -> void*
{
	// make the header ready
	rtpheader.version = 2;
	rtpheader.padding = 0;
	rtpheader.extension = 0;
	rtpheader.csrc_count = 0;
	rtpheader.marker = 0;
	rtpheader.payload_type = this->payloadType;
	rtpheader.sequence_number = this->sequenceNr++;
	rtpheader.timestamp = timestamp;
	rtpheader.ssrc = this->ssrc;

	// write into buffer

	memcpy((char*)newRTPPackage_Buffer, &rtpheader, rtp_header_size);
	memcpy((char*)(newRTPPackage_Buffer)+rtp_header_size, data, dataSize);

	return newRTPPackage_Buffer;
}

auto RTPPackage::getDataFromRTPPackage(void *rtpPackage) -> void*
{
	memcpy((char*)(readAudioDataFromRTPPackage_Buffer), (char*)rtpPackage + rtp_header_size, dataSize);
	return readAudioDataFromRTPPackage_Buffer;
}

auto RTPPackage::getHeaderFromRTPPackage(void *rtpPackage) -> void*
{
	memcpy(readRTPHeaderFromRTPPackage_Buffer, rtpPackage, dataSize);
	return readRTPHeaderFromRTPPackage_Buffer;
}

auto RTPPackage::getDataFromRTPPackage() -> void*
{
	memcpy((char*)(readAudioDataFromRTPPackage_Buffer), (char*)rtpPackageRecv_Buffer + rtp_header_size, dataSize);
	return readAudioDataFromRTPPackage_Buffer;
}

auto RTPPackage::getHeaderFromRTPPackage() -> void*
{
	memcpy(readRTPHeaderFromRTPPackage_Buffer, rtpPackageRecv_Buffer, dataSize);
	return readRTPHeaderFromRTPPackage_Buffer;
}

unsigned int RTPPackage::getRandomNumber()
{
	return this->randomGenerator();
}

unsigned int RTPPackage::getTimestamp()
{
    //start timestamp should be a random number
    return this->randomGenerator(); 
}

unsigned int RTPPackage::getAudioSourceId()
{
    //SSRC should be a random number
    return this->randomGenerator();
}

auto RTPPackage::getPacketSizeRTPPackage() -> unsigned int
{
	return this->dataSize + this->rtp_header_size;
}

auto RTPPackage::getRecvBuffer() -> void*
{
	return this->rtpPackageRecv_Buffer;
}
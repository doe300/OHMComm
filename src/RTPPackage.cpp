/* 
 * File:   RTPPackage.cpp
 * Author: daniel
 * 
 * Created on May 02, 2015, 13:03 PM
 */

#include "RTPPackage.h"


auto RTPPackage::getNewRTPPackage(void* data) -> void*
{
	// make the header ready
	rtpheader.version = 2;
	rtpheader.padding = 0;
	rtpheader.extension = 0;
	rtpheader.csrc_count = 0;
	rtpheader.marker = 0;
	rtpheader.payload_type = this->payloadType;
	rtpheader.sequence_number = this->sequenceNr++;
	rtpheader.timestamp = timestamp++; // TODO, get current timestamp
	rtpheader.ssrc = 0;

	// write into buffer

	memcpy((char*)rtpPackageBuffer, &rtpheader, rtp_header_size);
	memcpy((char*)(rtpPackageBuffer) + rtp_header_size, data, dataSize);

	return rtpPackageBuffer;
}

RTPPackage::RTPPackage(unsigned int dataSize, unsigned int rtp_header_size)
{
	this->dataSize = dataSize;
	this->rtp_header_size = rtp_header_size;
	rtpPackageBuffer = new char[dataSize + rtp_header_size];
	rtpReadDataBuffer = new char[dataSize];
	rtpReadHeaderBuffer = new char[rtp_header_size];


	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 tmp(seed1);
	this->randomGenerator = tmp;

	
	sequenceNr = getRandomNumber();
	timestamp = getTimestamp();
	ssrc = getAudioSourceId();
	// TODO, payloadType not correct
	payloadType = PayloadType::L16_2;
}

auto RTPPackage::getDataFromRTPPackage(void *rtpPackage) -> void*
{
	memcpy((char*)(rtpReadDataBuffer), (char*)rtpPackage + rtp_header_size, dataSize);
	return rtpReadDataBuffer;
}

auto RTPPackage::getHeaderFromRTPPackage(void *rtpPackage) -> void*
{
	memcpy(rtpReadHeaderBuffer, rtpPackage, dataSize);
	return rtpReadHeaderBuffer;
}

unsigned int RTPPackage::getRandomNumber()
{
	return this->randomGenerator();
}

//TODO Implementation incomplete
unsigned int RTPPackage::getTimestamp()
{
	return 5; 
}

//TODO Implementation incomplete
unsigned int RTPPackage::getAudioSourceId()
{
	return 5;
}
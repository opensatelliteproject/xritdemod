/*
 * CFileFrontend.cpp
 *
 *  Created on: 31/01/2017
 *      Author: Lucas Teske
 */

#include "CFileFrontend.h"

#include <cstdio>
#include <iostream>

#define BUFFERSIZE 65535

namespace SatHelper {

const std::string CFileFrontend::frontendName("CFile Frontend");
const std::vector<uint32_t> CFileFrontend::availableSampleRates;

CFileFrontend::CFileFrontend(const std::string &filename) :
		mainThread(NULL), filename(filename), sampleRate(0), centerFrequency(0) {
	sampleBuffer.resize(BUFFERSIZE);
	sampleBufferPtr = &sampleBuffer[0];
}

CFileFrontend::~CFileFrontend() {

}

void CFileFrontend::threadLoop() {
	FILE *f = fopen(filename.c_str(), "rb");
	float fPeriod = BUFFERSIZE / (float)sampleRate;
	std::chrono::milliseconds period((uint32_t) round(fPeriod * 1000));

	if (f == NULL) {
		std::cerr << "Error opening file" << filename << std::endl;
		this->running = false;
		return;
	}

	t0 = std::chrono::high_resolution_clock::now() - period;

	while (running) {
		if ((std::chrono::high_resolution_clock::now() - t0) >= period) {
			int samplesRead = fread(sampleBufferPtr, sizeof(std::complex<float>), BUFFERSIZE, f);
			if (cb != NULL) {
				cb(sampleBufferPtr, samplesRead, FRONTEND_SAMPLETYPE_FLOATIQ);
			}

			if (samplesRead == 0) {
				std::cerr << "EOF" << std::endl;
				running = false;
			}
			t0 = std::chrono::high_resolution_clock::now();
		}
	}

	fclose(f);
}

uint32_t CFileFrontend::SetSampleRate(uint32_t sampleRate) {
	return this->sampleRate = sampleRate;
}

uint32_t CFileFrontend::SetCenterFrequency(uint32_t centerFrequency) {
	return this->centerFrequency = centerFrequency;
}

const std::vector<uint32_t>& CFileFrontend	::GetAvailableSampleRates() {
	return availableSampleRates;
}

void CFileFrontend::Start() {
	this->running = true;
	mainThread = new std::thread(&CFileFrontend::threadLoop, this);
}

void CFileFrontend::Stop() {
	if (this->running) {
		this->running = false;
		this->mainThread->join();
	}
}

void CFileFrontend::SetAGC(bool agc) {

}

void CFileFrontend::SetLNAGain(uint8_t value) {

}

void CFileFrontend::SetVGAGain(uint8_t value) {

}

void CFileFrontend::SetMixerGain(uint8_t value) {

}

uint32_t CFileFrontend::GetCenterFrequency() {
	return centerFrequency;
}

const std::string &CFileFrontend::GetName() {
	return frontendName;
}

uint32_t CFileFrontend::GetSampleRate() {
	return sampleRate;
}

void CFileFrontend::SetSamplesAvailableCallback(
		std::function<void(void*data, int length, int type)> cb) {
	this->cb = cb;
}

} /* namespace SatHelper */

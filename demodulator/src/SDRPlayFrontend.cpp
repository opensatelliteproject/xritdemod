/*
 * SDRPlayFrontend.cpp
 *
 *  Created on: 27/02/2017
 *      Author: Lucas Teske
 */

#ifdef NON_FREE
#include "SDRPlayFrontend.h"
#include <mirsdrapi-rsp.h>
#include <SatHelper/exceptions.h>

const std::string SDRPlayFrontend::FrontendName = "SDRPlay OSP Plugin v0.1";
const std::vector<uint32_t> SDRPlayFrontend::supportedSampleRates = {
		2000000, 2500000, 3000000, 4000000, 5000000, 6000000, 7000000, 8000000, 9000000, 10000000
};

void SDRPlayFrontend::internalCallback(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, void *cbContext) {
	SDRPlayFrontend *ctx = (SDRPlayFrontend *)cbContext;

	if (ctx->bufferLength < numSamples) {
		if (ctx->buffer != NULL) {
			delete[] ctx->buffer;
		}
		ctx->buffer = new float[numSamples*2];
		ctx->bufferLength = numSamples;
	}

	for (unsigned int i=0; i<numSamples; i++) {
		ctx->buffer[i*2+0] = xi[i] / 16384.f;
		ctx->buffer[i*2+1] = xq[i] / 16384.f;
	}

	if (ctx->cb != NULL) {
		ctx->cb(ctx->buffer, numSamples, FRONTEND_SAMPLETYPE_FLOATIQ);
	}
}

void SDRPlayFrontend::internalCallbackGC(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext) {
	// Not sure what I should do here.
	std::cout << "Gains changed to " << gRdB << " - " << lnaGRdB << std::endl;
}


void SDRPlayFrontend::Initialize() {

	float ver;

	mir_sdr_ApiVersion(&ver);

	if (ver != MIR_SDR_API_VERSION) {
		std::cerr << "The library version and program doesn't match. Expected API: " << MIR_SDR_API_VERSION << " got " << ver << std::endl;
		throw SatHelperException("Incorrect library version");
	}

	// Set 1st LO frequency to 120MHz
	if (mir_sdr_SetParam(101, 24576000) != mir_sdr_Success) {
		std::cerr << "Cannot set SDRPLay 1st LO to 120MHz" << std::endl;
		throw SatHelperException("Cannot set SDRPLay 1st LO to 120MHz");
	}

	// Disable decimation on SDRPlay
	if (mir_sdr_DecimateControl(0, 1, 0) != mir_sdr_Success) {
		std::cerr << "Cannot set decimation options for SDRPlay" << std::endl;
		throw SatHelperException("Cannot set decimation options for SDRPlay");
	}
}

void SDRPlayFrontend::DeInitialize() {

}

SDRPlayFrontend::SDRPlayFrontend() {
	this->samplesPerPacket = 0;
	this->gRdB = 40;
	this->gRdBsystem = 83;
	this->sampleRate = 10000000;
	this->centerFrequency = 106300000;
	this->buffer = NULL;
	this->bufferLength = 0;
}

SDRPlayFrontend::~SDRPlayFrontend() {
	if (this->buffer != NULL) {
		delete[] this->buffer;
	}
}

uint32_t SDRPlayFrontend::SetSampleRate(uint32_t sampleRate) {
	return this->sampleRate = sampleRate;
}

uint32_t SDRPlayFrontend::SetCenterFrequency(uint32_t centerFrequency) {
	return this->centerFrequency = centerFrequency;
}

const std::vector<uint32_t>& SDRPlayFrontend::GetAvailableSampleRates() {
	return supportedSampleRates;
}

void SDRPlayFrontend::SetAntenna(int antenna) {
	if (mir_sdr_AmPortSelect(antenna) != mir_sdr_Success) {
		std::cerr << "Error setting antenna to port " << antenna << std::endl;
		throw SatHelperException("SDRPlay Invalid Antenna");
	}
}

void SDRPlayFrontend::Start() {
	double fsMHz = sampleRate / 1e6;
	double rfMHz = centerFrequency / 1e6;
#ifdef DEBUG_MODE
	mir_sdr_DebugEnable(1);
#endif
	int err = mir_sdr_StreamInit(&gRdB, fsMHz, rfMHz, mir_sdr_BW_5_000, mir_sdr_IF_Zero, 4, &gRdBsystem, mir_sdr_USE_RSP_SET_GR, &samplesPerPacket, &SDRPlayFrontend::internalCallback, &SDRPlayFrontend::internalCallbackGC, (void *)this);
	if (err != mir_sdr_Success) {
		std::cerr << "Error starting SDRPlay: ";
		switch(err) {
		case mir_sdr_AlreadyInitialised:
			std:: cerr << "Already Initialized" << std::endl;
			throw SatHelperException("SDRPlay Already Initialized");
			break;
		case mir_sdr_InvalidParam:
			std:: cerr << "Invalid Parameter" << std::endl;
			throw SatHelperException("SDRPlay Invalid Parameter");
			break;
		case mir_sdr_OutOfRange:
			std:: cerr << "Out of Range" << std::endl;
			throw SatHelperException("SDRPlay Out of Range");
			break;
		case mir_sdr_HwError:
			std:: cerr << "Hardware Error" << std::endl;
			throw SatHelperException("SDRPlay Hardware Error");
			break;
		case mir_sdr_Fail:
			std:: cerr << "Failed to access device" << std::endl;
			throw SatHelperException("SDRPlay Failed to Access Device");
			break;
		default:
			std:: cerr << "Unknown Error: " << err << std::endl;
			throw SatHelperException("SDRPlay Unknown Error");
			break;
		}
	}

}

void SDRPlayFrontend::Stop() {
	mir_sdr_StreamUninit();
}

void SDRPlayFrontend::SetAGC(bool agc) {
	mir_sdr_AgcControl(agc ? mir_sdr_AGC_100HZ : mir_sdr_AGC_DISABLE, -30, 0, 0, 0, 0, 1);
}

void SDRPlayFrontend::SetLNAGain(uint8_t value) {
	gRdB = value;
}

void SDRPlayFrontend::SetVGAGain(uint8_t value) {
	std::cerr << "Not implemented for SDRPlay" << std::endl;
}

void SDRPlayFrontend::SetMixerGain(uint8_t value) {
	std::cerr << "Not implemented for SDRPlay" << std::endl;
}

uint32_t SDRPlayFrontend::GetCenterFrequency() {
	return centerFrequency;
}

const std::string &SDRPlayFrontend::GetName() {
	return FrontendName;
}

uint32_t SDRPlayFrontend::GetSampleRate() {
	return sampleRate;
}

void SDRPlayFrontend::SetSamplesAvailableCallback(std::function<void(void*data, int length, int type)> cb) {
	this->cb = cb;
}

#endif

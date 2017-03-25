/*
 * HackRFFrontend.cpp
 *
 *  Created on: 26/02/2017
 *      Author: Lucas Teske
 */

#include "HackRFFrontend.h"
#include <SatHelper/exceptions.h>
#include <iostream>
#include <cmath>


const std::string HackRFFrontend::FrontendName = "HackRF OSP Plugin";
const std::vector<uint32_t> HackRFFrontend::supportedSampleRates = {
		8000000, 10000000, 12500000, 16000000, 20000000
};

void HackRFFrontend::Initialize() {
	int err = hackrf_init();
	if (err != HACKRF_SUCCESS) {
		const char *msg = hackrf_error_name((hackrf_error)err);
		std::cerr << "Error initializing HackRF: " << msg << std::endl;
	}
}

void HackRFFrontend::DeInitialize() {

}


int HackRFFrontend::callback(hackrf_transfer* transfer) {
	HackRFFrontend *ctx = (HackRFFrontend *)transfer->rx_ctx;
	if (transfer->valid_length > ctx->bufferLength) {
		if (ctx->buffer != NULL) {
			delete[] ctx->buffer;
		}

		ctx->buffer = new float[transfer->valid_length];
		ctx->bufferLength = transfer->valid_length;
	}

	for (int i=0; i<transfer->valid_length; i++) {
		ctx->buffer[i] = ctx->lut[transfer->buffer[i]];
		if (i%1) {
			ctx->qavg += ctx->alpha * (ctx->buffer[i] - ctx->qavg);
			ctx->buffer[i] -= ctx->qavg;
		} else {
			ctx->iavg += ctx->alpha * (ctx->buffer[i] - ctx->iavg);
			ctx->buffer[i] -= ctx->iavg;
		}
	}

	ctx->cb(ctx->buffer, transfer->valid_length / 2, FRONTEND_SAMPLETYPE_FLOATIQ);

	return 0;
}


HackRFFrontend::HackRFFrontend(int deviceNumber) {
	hackrf_device_list_t *devices = hackrf_device_list();
	if (deviceNumber >= devices->devicecount) {
		std::cerr << "There is no such device as id " << deviceNumber << std::endl;
		throw SatHelperException("No such device");
	}
	this->deviceNumber = deviceNumber;
	device = NULL;
	hackrf_device_list_free(devices);
	lnaGain = 0;
	vgaGain = 0;
	mixerGain = 0;
	centerFrequency = 106300000;
	sampleRate = 8000000;
	buffer = NULL;
	bufferLength = 0;
	alpha = 1.f - exp(-1.0 / (sampleRate * 0.05f));
	iavg = 0;
	qavg = 0;
	for (int i = 0; i < 256; i++) {
		lut[i] = (i - 128) * (1.f / 127.f);
	}
}

HackRFFrontend::~HackRFFrontend() {
	if (buffer != NULL) {
		delete[] buffer;
	}
}

uint32_t HackRFFrontend::SetSampleRate(uint32_t sampleRate) {
	if (device != NULL) {
		double sr = sampleRate / 1e6;
		int err = hackrf_set_sample_rate(device, sr);
		if (err != HACKRF_SUCCESS) {
			const char *msg = hackrf_error_name((hackrf_error)err);
			std::cerr << "Error setting sample rate HackRF: " << msg << std::endl;
		}
	}

	this->sampleRate = sampleRate;;
	return sampleRate;
}

uint32_t HackRFFrontend::SetCenterFrequency(uint32_t centerFrequency) {
	if (device != NULL) {
		int err = hackrf_set_freq(device, centerFrequency);
		if (err != HACKRF_SUCCESS) {
			const char *msg = hackrf_error_name((hackrf_error)err);
			std::cerr << "Error setting center frequency HackRF: " << msg << std::endl;
		}
	}

	this->centerFrequency = centerFrequency;
	return centerFrequency;
}

const std::vector<uint32_t>& HackRFFrontend::GetAvailableSampleRates() {
	return supportedSampleRates;
}

void HackRFFrontend::Start() {
	alpha = 1.f - exp(-1.0 / (sampleRate * 0.05f));
	iavg = 0;
	qavg = 0;
	hackrf_device_list_t *devices = hackrf_device_list();

	std::cout << "Getting HackRF Devices" << std::endl;
	int err = hackrf_device_list_open(devices, deviceNumber, &device);
	if (err != HACKRF_SUCCESS) {
		const char *msg = hackrf_error_name((hackrf_error)err);
		std::cerr << "Error listing devices: " << msg << std::endl;
	}

	std::cout << "Disabling Input LNA for safety." << std::endl;

	err = hackrf_set_amp_enable(device, 0);
	if (err != HACKRF_SUCCESS) {
		const char *msg = hackrf_error_name((hackrf_error)err);
		std::cerr << "Error disabling input LNA: " << msg << std::endl;
	}

	std::cout << "Setting LNA Gain to " << (int)lnaGain << std::endl;
	SetLNAGain(lnaGain);
	std::cout << "Setting VGA Gain to " << (int)vgaGain << std::endl;
	SetVGAGain(vgaGain);
	std::cout << "Setting Sample Rate to " << sampleRate << std::endl;
	SetSampleRate(sampleRate);

	uint32_t bwfilter = hackrf_compute_baseband_filter_bw(4000000);
	std::cout << "Computed Bandwidth Filter: " << bwfilter << std::endl;

	err = hackrf_set_baseband_filter_bandwidth(device, bwfilter);

	if (err != HACKRF_SUCCESS) {
		const char *msg = hackrf_error_name((hackrf_error)err);
		std::cerr << "Error setting input bandwidth: " << msg << std::endl;
	}

	err = hackrf_start_rx(device, HackRFFrontend::callback, this);
	if (err != HACKRF_SUCCESS) {
		const char *msg = hackrf_error_name((hackrf_error)err);
		std::cerr << "Error initializing HackRF: " << msg << std::endl;
	}

	std::cout << "Setting Center Frequency to " << centerFrequency << std::endl;
	SetCenterFrequency(centerFrequency);

	hackrf_device_list_free(devices);
}

void HackRFFrontend::Stop() {
	if (device != NULL) {
		hackrf_close((hackrf_device *)device);
	}
}

void HackRFFrontend::SetAGC(bool agc) {
	if (agc == true) {
		std::cout << "AGC not implemented for HackRF. Please set the gains manually." << std::endl;
		throw SatHelperException("No hardware AGC available");
	}
}

void HackRFFrontend::SetLNAGain(uint8_t value) {
	if (device != NULL) {
		int err = hackrf_set_lna_gain(device, value);
		if (err != HACKRF_SUCCESS) {
			const char *msg = hackrf_error_name((hackrf_error)err);
			std::cerr << "Error setting LNA Gain for HackRF: " << msg << std::endl;
		}
	}
	lnaGain = value;
}

void HackRFFrontend::SetVGAGain(uint8_t value) {
	if (device != NULL) {
		int err = hackrf_set_vga_gain(device, value);
		if (err != HACKRF_SUCCESS) {
			const char *msg = hackrf_error_name((hackrf_error)err);
			std::cerr << "Error setting VGA Gain for HackRF: " << msg << std::endl;
		}
	}
	vgaGain = value;
}

void HackRFFrontend::SetMixerGain(uint8_t value) {
	std::cerr << "HackRF does not have mixer gain." << std::endl;
}

uint32_t HackRFFrontend::GetCenterFrequency() {
	return centerFrequency;
}

const std::string &HackRFFrontend::GetName() {
	return FrontendName;
}

uint32_t HackRFFrontend::GetSampleRate() {
	return sampleRate;
}

void HackRFFrontend::SetSamplesAvailableCallback(std::function<void(void*data, int length, int type)> cb) {
	this->cb = cb;
}

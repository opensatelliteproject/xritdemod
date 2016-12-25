//============================================================================
// Name        : GOES LRIT Demodulator
// Author      : Lucas Teske
// Version     : 1.0
// Copyright   : Copyright 2016
// Description : GOES LRIT Demodulator - Open Satellite Project
//============================================================================

#include <cstdio>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <cstring>

#include "AirspyDevice.h"
#include "SymbolManager.h"
#include "sathelper.h"

using namespace OpenSatelliteProject;
using namespace SatHelper;

// LRIT Options
#define CENTER_FREQUENCY 1691000000
#define LOOP_ORDER 2
#define SYMBOL_RATE 293883
#define RRC_ALPHA 0.5
#define RRC_TAPS 361
#define PLL_ALPHA 0.025
#define CLOCK_ALPHA 0.0037
#define CLOCK_MU 0.5
#define CLOCK_OMEGA_LIMIT 0.005
#define CLOCK_GAIN_OMEGA (CLOCK_ALPHA * CLOCK_ALPHA) / 4.0

// Use 2 for 2.5Msps on Airspy R2 and 3.0Msps on Airspy Mini
#define BASE_DECIMATION 2

AGC *agc;
CostasLoop *costasLoop;
ClockRecovery *clockRecovery;
FirFilter *rrcFilter;
FirFilter *decimator;
SymbolManager symbolManager;

bool running;

// In number of samples
int sampleDataLength = 0;

// Flow Buffers
std::complex<float> *sampleData = NULL;
std::complex<float> *decimatorData = NULL;
std::complex<float> *agcData = NULL;
std::complex<float> *rrcData = NULL;
std::complex<float> *costasData = NULL;
std::complex<float> *clockRecoveryData = NULL;

int tttc = 0;

void onSamplesAvailable(void *fdata, int length) {
	float *data = (float *) fdata;
	if (sampleDataLength != length) {
		std::cout << "Allocating Sample Buffer with size " << length
				<< " (it was " << sampleDataLength << " before)" << std::endl;
		if (sampleData != NULL) {
			delete sampleData;
		}

		if (agcData != NULL) {
			delete agcData;
		}

		if (rrcData != NULL) {
			delete rrcData;
		}

		if (costasData != NULL) {
			delete costasData;
		}

		if (clockRecoveryData != NULL) {
			delete clockRecoveryData;
		}

		if (decimatorData != NULL) {
			delete decimatorData;
		}

		sampleData = new std::complex<float>[length];
		agcData = new std::complex<float>[length / BASE_DECIMATION];
		costasData = new std::complex<float>[length / BASE_DECIMATION];
		clockRecoveryData = new std::complex<float>[length / BASE_DECIMATION];
		rrcData = new std::complex<float>[length / BASE_DECIMATION];
		decimatorData = new std::complex<float>[length / BASE_DECIMATION];
		sampleDataLength = length;
	}

	// Convert Interleaved Float IQ to Complex
	for (int i = 0; i < length; i++) {
		sampleData[i] = std::complex<float>(data[i * 2], data[i * 2 + 1]);
	}

	// Decimation / Lowpass
	length /= BASE_DECIMATION;
	decimator->Work(sampleData, decimatorData, length);

	// Automatic Gain Control
	agc->Work(decimatorData, agcData, length);

	// Filter
	rrcFilter->Work(agcData, rrcData, length);

	// Costas Loop
	costasLoop->Work(rrcData, costasData, length);

	// Clock Recovery
	int symbols = clockRecovery->Work(costasData, clockRecoveryData, length);

	symbolManager.add(clockRecoveryData, symbols);

	FILE *f = fopen("test.bin", "wb");
	fwrite(clockRecoveryData, sizeof(std::complex<float>), symbols, f);
	fclose(f);

	// Temporary Testing
	tttc++;
	if (tttc == 100) {
		running = false;
	}
}

int main(int argc, char **argv) {

	AirspyDevice::Initialize();
	AirspyDevice airspy;

	bool isMini = false;

	std::vector<uint64_t> sampleRates = airspy.GetAvailableSampleRates();
	for (uint64_t sampleRate : sampleRates) {
		if (sampleRate == 3000000) {
			isMini = true;
			airspy.SetSampleRate(sampleRate);
			break;
		} else if (sampleRate == 2500000) {
			isMini = false;
			airspy.SetSampleRate(sampleRate);
			break;
		}
	}

	std::cout << "Airspy " << (isMini ? "Mini " : "R2 ")
			<< "detected. Sample rate set to " << airspy.GetSampleRate()
			<< std::endl;

	airspy.SetSamplesAvailableCallback(onSamplesAvailable);

	float circuitSampleRate = airspy.GetSampleRate() / ((float) BASE_DECIMATION);
	float sps = circuitSampleRate / ((float) SYMBOL_RATE);

	std::cout << "Samples per Symbol: " << sps << std::endl;
	std::cout << "Circuit Sample Rate: " << circuitSampleRate << std::endl;

	std::vector<float> rrcTaps = Filters::RRC(1, circuitSampleRate, SYMBOL_RATE, RRC_ALPHA, RRC_TAPS);
	std::vector<float> decimatorTaps = Filters::lowPass(1, airspy.GetSampleRate(), circuitSampleRate / 2, 100e3, FFTWindows::WindowType::BLACKMAN_HARRIS, 6.76);

	decimator = new FirFilter(BASE_DECIMATION, decimatorTaps);
	agc = new AGC(0.01, 0.5, 1, 4000);
	costasLoop = new CostasLoop(PLL_ALPHA, LOOP_ORDER);
	clockRecovery = new ClockRecovery(sps, CLOCK_GAIN_OMEGA, CLOCK_MU, CLOCK_ALPHA, CLOCK_OMEGA_LIMIT);
	rrcFilter = new FirFilter(1, rrcTaps);

	airspy.SetCenterFrequency(CENTER_FREQUENCY);
	airspy.SetAGC(false);
	airspy.SetLNAGain(15);
	airspy.SetVGAGain(15);
	airspy.SetMixerGain(15);

	std::cout << "Starting Airspy" << std::endl;
	airspy.Start();
	running = true;

	while (running) {

	}

	std::cout << "Stopping Airspy" << std::endl;
	airspy.Stop();

	std::cout << "Closing" << std::endl;

	delete agc;
	delete costasLoop;
	delete clockRecovery;
	delete rrcFilter;

	return 0;
}

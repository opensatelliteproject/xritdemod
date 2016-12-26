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
#define RRC_TAPS 63
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
std::complex<float> *buffer0 = NULL;
std::complex<float> *buffer1 = NULL;

void onSamplesAvailable(void *fdata, int length) {
	float *data = (float *) fdata;
	if (sampleDataLength != length) {
		std::cout << "Allocating Sample Buffer with size " << length
				<< " (it was " << sampleDataLength << " before)" << std::endl;
		if (buffer0 != NULL) {
			delete buffer0;
		}

		if (buffer1 != NULL) {
			delete buffer1;
		}

		buffer0 = new std::complex<float>[length];
		buffer1 = new std::complex<float>[length];
		sampleDataLength = length;
	}

	// Convert Interleaved Float IQ to Complex
	for (int i = 0; i < length; i++) {
		buffer0[i] = std::complex<float>(data[i * 2], data[i * 2 + 1]);
	}

	// Decimation / Lowpass
	length /= BASE_DECIMATION;
	decimator->Work(buffer0, buffer1, length);

	// Automatic Gain Control
	agc->Work(buffer1, buffer0, length);

	// Filter
	rrcFilter->Work(buffer0, buffer1, length);

	// Costas Loop
	costasLoop->Work(buffer1, buffer0, length);

	// Clock Recovery
	int symbols = clockRecovery->Work(buffer0, buffer1, length);

	symbolManager.add(buffer1, symbols);
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
	std::cout << "Low Pass Decimator Cut Frequency: " << circuitSampleRate / 2 << std::endl;

	std::vector<float> rrcTaps = Filters::RRC(1, circuitSampleRate, SYMBOL_RATE, RRC_ALPHA, RRC_TAPS);
	std::vector<float> decimatorTaps = Filters::lowPass(1, airspy.GetSampleRate(), circuitSampleRate, 100e3, FFTWindows::WindowType::HAMMING, 6.76);

	decimator = new FirFilter(BASE_DECIMATION, decimatorTaps);
	agc = new AGC(0.01, 0.5, 1, 4000);
	costasLoop = new CostasLoop(PLL_ALPHA, LOOP_ORDER);
	clockRecovery = new ClockRecovery(sps, CLOCK_GAIN_OMEGA, CLOCK_MU, CLOCK_ALPHA, CLOCK_OMEGA_LIMIT);
	rrcFilter = new FirFilter(1, rrcTaps);

	airspy.SetCenterFrequency(CENTER_FREQUENCY);
	airspy.SetAGC(false);
	airspy.SetMixerGain(15);
	airspy.SetLNAGain(15);
	airspy.SetVGAGain(15);

	std::cout << "Starting Airspy" << std::endl;
	airspy.Start();
	running = true;

	while (running) {
		int siq = symbolManager.symbolsInQueue();
		if (siq > 0) {
			symbolManager.process();
		}
		usleep(10);	// Let's not waste CPU time
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

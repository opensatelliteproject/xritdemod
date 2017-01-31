//============================================================================
// Name        : GOES xRIT Demodulator
// Author      : Lucas Teske
// Version     : 1.0
// Copyright   : Copyright 2016
// Description : GOES xRIT Demodulator - Open Satellite Project
//============================================================================

#include <cstdio>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <cstring>
#include <SatHelper/sathelper.h>
#include "FrontendDevice.h"
#include "AirspyDevice.h"
#include "CFileFrontend.h"
#include "SymbolManager.h"
#include "SampleFIFO.h"

using namespace OpenSatelliteProject;
using namespace SatHelper;

#include "Parameters.h"


FrontendDevice *device;
AGC *agc;
CostasLoop *costasLoop;
ClockRecovery *clockRecovery;
FirFilter *rrcFilter;
FirFilter *decimator;
SymbolManager symbolManager;
SampleFIFO samplesFifo(FIFO_SIZE);

int baseDecimation;

bool running;

// In number of samples
int sampleDataLength = 0;

// Flow Buffers
std::complex<float> *buffer0 = NULL;
std::complex<float> *buffer1 = NULL;

uint32_t startTime = 0;

void onSamplesAvailable(void *fdata, int length, int type) {
	if (type == FRONTEND_SAMPLETYPE_FLOATIQ) {
		samplesFifo.addSamples((float *) fdata, length * 2);
	} else {
		std::cerr << "Unknown sample type: " << type << std::endl;
	}
}

void checkAndResizeBuffers(int length) {
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
}

inline void swapBuffers(std::complex<float> **a, std::complex<float> **b) {
	std::complex<float> *c = *b;
	*b = *a;
	*a = c;
}

void processSamples() {
	int length;
	std::complex<float> *ba, *bb;

	if (samplesFifo.isOverflow()) {
		std::cerr << "Input Samples Fifo is overflowing!" << std::endl;
	}

	if (!samplesFifo.containsSamples()) {
		// No data
		return;
	}

	length = samplesFifo.size() / 2;
	checkAndResizeBuffers(length);
	// Do unsafe locks and copy queue for the buffer;
	samplesFifo.unsafe_lockMutex();

	// Convert Interleaved Float IQ to Complex
	for (int i=0; i<length; i++) {
		float I = samplesFifo.unsafe_takeSample();
		float Q = samplesFifo.unsafe_takeSample();
		buffer0[i] = std::complex<float>(I, Q);
	}

	samplesFifo.unsafe_unlockMutex();

	ba = buffer0;
	bb = buffer1;

	// Decimation / Lowpass
	if (baseDecimation > 1) {
		length /= baseDecimation;
		decimator->Work(ba, bb, length);
		swapBuffers(&ba, &bb);
	}

	// Automatic Gain Control
	agc->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	// Filter
	rrcFilter->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	// Costas Loop
	costasLoop->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	// Clock Recovery
	int symbols = clockRecovery->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	symbolManager.add(ba, symbols);
}

void symbolLoopFunc() {
	while (running) {
		processSamples();
		usleep(1);	// Let's not waste CPU time
	}
}

void setLRITMode(ConfigParser &parser, bool normal) {
	parser[CFG_SYMBOL_RATE] = std::string(QUOTE(LRIT_SYMBOL_RATE));
	parser[CFG_MODE] = std::string("lrit");
	parser[CFG_RRC_ALPHA] = std::string(QUOTE(LRIT_RRC_ALPHA));
	if (normal) {
		parser[CFG_FREQUENCY] = std::string(QUOTE(LRIT_CENTER_FREQUENCY));
		parser[CFG_SAMPLE_RATE] = std::string(QUOTE(DEFAULT_SAMPLE_RATE));
		parser[CFG_DECIMATION] = std::string(QUOTE(DEFAULT_DECIMATION));
	}
}

void setHRITMode(ConfigParser &parser, bool normal) {
	parser[CFG_SYMBOL_RATE] = std::string(QUOTE(HRIT_SYMBOL_RATE));
	parser[CFG_MODE] = std::string("hrit");
	parser[CFG_RRC_ALPHA] = std::string(QUOTE(HRIT_RRC_ALPHA));
	if (normal) {
		parser[CFG_FREQUENCY] = std::string(QUOTE(HRIT_CENTER_FREQUENCY));
		parser[CFG_SAMPLE_RATE] = std::string(QUOTE(DEFAULT_SAMPLE_RATE));
		parser[CFG_DECIMATION] = std::string(QUOTE(DEFAULT_DECIMATION));
	}
}

void setDefaults(ConfigParser &parser) {
	setLRITMode(parser, true);
	parser[CFG_AGC] = "true";
	parser[CFG_LNA_GAIN] = std::string(QUOTE(DEFAULT_LNA_GAIN));
	parser[CFG_MIXER_GAIN] = std::string(QUOTE(DEFAULT_MIX_GAIN));
	parser[CFG_VGA_GAIN] = std::string(QUOTE(DEFAULT_VGA_GAIN));
	parser[CFG_DEVICE_TYPE] = std::string("airspy");
	parser.SaveFile();
}

int main(int argc, char **argv) {
	ConfigParser parser("xritdemod.cfg");
	float rrcAlpha;
	uint32_t symbolRate, centerFrequency, sampleRate;
	uint8_t lnaGain = 15, vgaGain = 15, mixerGain = 15;
	bool agcEnable = false;

	if (!parser.LoadFile()) {
		// Add defaults to LRIT
		std::cout << "No config file found. Defaulting to LRIT and creating config file." << std::endl;
		std::cout << "The config file will be created with both \"mode\" and specific parameters. " << std::endl;
		std::cout << "But if mode is specified, the other arguments will be ignore. The created file is just an example." << std::endl;
		setDefaults(parser);
	}

	if (parser.hasKey(CFG_MODE)) {
		if (parser[CFG_MODE] == "lrit") {
			std::cout << "Selected LRIT mode. Ignoring parameters from config file." << std::endl;
			setLRITMode(parser, false);
		} else if (parser[CFG_MODE] == "hrit") {
			std::cout << "Selected HRIT mode. Ignoring parameters from config file." << std::endl;
			setHRITMode(parser, false);
		} else {
			std::cerr << "Invalid mode specified: " << parser[CFG_MODE] << std::endl;
			return 1;
		}
	}

	if (parser.hasKey(CFG_SYMBOL_RATE)) {
		symbolRate = parser.getUInt(CFG_SYMBOL_RATE);
	} else {
		std::cerr << "Field \"symbolRate\" is missing on config file." << std::endl;
		return 1;
	}

	if (parser.hasKey(CFG_RRC_ALPHA)) {
		rrcAlpha = parser.getFloat(CFG_RRC_ALPHA);
	} else {
		std::cerr << "Field \"rrcAlpha\" is missing on config file." << std::endl;
		return 1;
	}

	if (parser.hasKey(CFG_FREQUENCY)) {
		centerFrequency = parser.getUInt(CFG_FREQUENCY);
	} else {
		std::cerr << "Field \"frequency\" is missing on config file." << std::endl;
		return 1;
	}

	if (parser.hasKey(CFG_SAMPLE_RATE)) {
		sampleRate = parser.getUInt(CFG_SAMPLE_RATE);
		std::cout << "Sample Rate: " << sampleRate << std::endl;
	} else {
		std::cerr << "Field \"sampleRate\" is missing on config file." << std::endl;
		return 1;
	}

	if (parser.hasKey(CFG_DECIMATION)) {
		baseDecimation = parser.getInt(CFG_DECIMATION);
	} else {
		std::cerr << "Field \"decimation\" is missing on config file." << std::endl;
		return 1;
	}

	if (parser.hasKey(CFG_LNA_GAIN)) {
		lnaGain = parser.getInt(CFG_LNA_GAIN);
	}

	if (parser.hasKey(CFG_MIXER_GAIN)) {
		mixerGain = parser.getInt(CFG_MIXER_GAIN);
	}

	if (parser.hasKey(CFG_VGA_GAIN)) {
		vgaGain = parser.getInt(CFG_VGA_GAIN);
	}

	if (parser.hasKey(CFG_AGC)) {
		agcEnable = parser.getBool(CFG_AGC);
		if (agcEnable) {
			lnaGain = 15;
			vgaGain = 15;
			mixerGain = 15;
		}
	}

	if (parser.hasKey(CFG_DEVICE_TYPE)) {
		if (parser[CFG_DEVICE_TYPE] == "airspy") {
			AirspyDevice::Initialize();
			device = new AirspyDevice();

			bool isMini = false;
			bool sampleRateSet = false;

			std::vector<uint32_t> sampleRates = device->GetAvailableSampleRates();
			for (uint32_t asSR : sampleRates) {
				if (asSR == sampleRate) {
					isMini = true;
					device->SetSampleRate(sampleRate);
					sampleRateSet = true;
					break;
				}
			}

			if (!sampleRateSet) {
				std::cerr << "Your device is not compatible with sampleRate \"" << sampleRate << "\"" << std::endl;
				return 1;
			}

			std::cout << "Airspy " << (isMini ? "Mini " : "R2 ")
					<< "detected. Sample rate set to " << device->GetSampleRate()
					<< std::endl;

		} else if (parser[CFG_DEVICE_TYPE] == "cfile") {
			if (!parser.hasKey(CFG_FILENAME)) {
				std::cerr << "Device Type defined as \"cfile\" but no \"filename\" specified." << std::endl;
				return 1;
			}
			device = new CFileFrontend(parser[CFG_FILENAME]);
			device->SetCenterFrequency(parser.getUInt(CFG_FREQUENCY));
			device->SetSampleRate(parser.getUInt(CFG_SAMPLE_RATE));
		} else if (parser[CFG_DEVICE_TYPE] == "wav") {
			std::cerr << "WAV Reader not implemented." << std::endl;
			return 1;
		}
	} else {
		std::cerr << "Input Device Type not specified in config file." << std::endl;
		return 1;
	}


	device->SetSamplesAvailableCallback(onSamplesAvailable);

	float circuitSampleRate = device->GetSampleRate() / ((float) baseDecimation);
	float sps = circuitSampleRate / ((float) symbolRate);

	std::cout << "Samples per Symbol: " << sps << std::endl;
	std::cout << "Circuit Sample Rate: " << circuitSampleRate << std::endl;
	std::cout << "Low Pass Decimator Cut Frequency: " << circuitSampleRate / 2 << std::endl;

	std::vector<float> rrcTaps = Filters::RRC(1, circuitSampleRate, symbolRate, rrcAlpha, RRC_TAPS);
	std::vector<float> decimatorTaps = Filters::lowPass(1, device->GetSampleRate(), circuitSampleRate / 2, 100e3, FFTWindows::WindowType::HAMMING, 6.76);

	decimator = new FirFilter(baseDecimation, decimatorTaps);
	agc = new AGC(AGC_RATE, AGC_REFERENCE, AGC_GAIN, AGC_MAX_GAIN);
	costasLoop = new CostasLoop(PLL_ALPHA, LOOP_ORDER);
	clockRecovery = new ClockRecovery(sps, CLOCK_GAIN_OMEGA, CLOCK_MU, CLOCK_ALPHA, CLOCK_OMEGA_LIMIT);
	rrcFilter = new FirFilter(1, rrcTaps);

	std::cout << "Center Frequency: " << (centerFrequency / 1000000.0) << " MHz" << std::endl;
	std::cout << "Automatic Gain Control: " << (agcEnable ? "Enabled" : "Disabled") << std::endl;
	if (!agcEnable) {
		std::cout << "	LNA Gain: " << lnaGain << std::endl;
		std::cout << "	VGA Gain: " << vgaGain << std::endl;
		std::cout << "	MIX Gain: " << mixerGain << std::endl;
	}

	device->SetCenterFrequency(centerFrequency);
	if (agcEnable) {
		device->SetAGC(true);
	} else {
		device->SetAGC(false);
		device->SetMixerGain(lnaGain);
		device->SetLNAGain(vgaGain);
		device->SetVGAGain(mixerGain);
	}

	std::cout << "Starting " << device->GetName() << std::endl;
	device->Start();
	running = true;

	std::thread symbolThread(&symbolLoopFunc);

	while (running) {
		int siq = symbolManager.symbolsInQueue();
		if (siq > 0) {
			symbolManager.process();
		}
		usleep(1);	// Let's not waste CPU time
	}

	std::cout << "Stopping Airspy" << std::endl;
	device->Stop();

	std::cout << "Stopping Symbol Processing Thread" << std::endl;
	symbolThread.join();

	std::cout << "Closing" << std::endl;

	delete agc;
	delete costasLoop;
	delete clockRecovery;
	delete rrcFilter;
	delete device;

	return 0;
}

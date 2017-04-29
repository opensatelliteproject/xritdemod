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
#include "RtlFrontend.h"
#include "CFileFrontend.h"
#include "SymbolManager.h"
#include "DiagManager.h"
#include "ExitHandler.h"
#include "SDRPlayFrontend.h"
#include "HackRFFrontend.h"
#include "SpyServerFrontend.h"

using namespace OpenSatelliteProject;
using namespace SatHelper;

// Uncomment this to passthrough the input samples to output
//#define DEBUG_PASSTHROUGH

#include "Parameters.h"

FrontendDevice *device = NULL;
AGC *agc = NULL;
CostasLoop *costasLoop = NULL;
ClockRecovery *clockRecovery = NULL;
FirFilter *rrcFilter = NULL;
FirFilter *decimator = NULL;
SymbolManager *symbolManager = NULL;
CircularBuffer<float> samplesFifo(FIFO_SIZE);
DiagManager *diagManager = NULL;

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
	} else if (type == FRONTEND_SAMPLETYPE_S16IQ) {
		int16_t *d = (int16_t *)fdata;
		samplesFifo.unsafe_lockMutex();
		for (int i=0; i<length*2; i++) {
			samplesFifo.unsafe_addSample(d[i] / 32768.f);
		}
		samplesFifo.unsafe_unlockMutex();
	}else if (type == FRONTEND_SAMPLETYPE_S8IQ) {
		int8_t *d = (int8_t *)fdata;
		samplesFifo.unsafe_lockMutex();
		for (int i=0; i<length*2; i++) {
			samplesFifo.unsafe_addSample(d[i] / 128.f);
		}
		samplesFifo.unsafe_unlockMutex();
	} else {
		std::cerr << "Unknown sample type: " << type << std::endl;
	}
}

void checkAndResizeBuffers(int length) {
	if (sampleDataLength < length) {
		std::cout << "Allocating Sample Buffer with size " << length
				<< " (it was " << sampleDataLength << " before)" << std::endl;
		if (buffer0 != NULL) {
			delete[] buffer0;
		}

		if (buffer1 != NULL) {
			delete[] buffer1;
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

#ifndef DEBUG_PASSTHROUGH
	// Filter
	rrcFilter->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	// Costas Loop
	costasLoop->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	// Clock Recovery
	int symbols = clockRecovery->Work(ba, bb, length);
	swapBuffers(&ba, &bb);

	symbolManager->add(ba, symbols);

	if (diagManager != NULL) {
		diagManager->addSamples((float *)ba, symbols < 1024 ? symbols : 1024);
	}
#else
	std::cout << ba[0] << " - writting " << length << std::endl;
	symbolManager->add((float *)ba, length*2);
#endif
}

void symbolLoopFunc() {
	while (running) {
		processSamples();
		std::this_thread::sleep_for(std::chrono::microseconds(1)); // Let's not waste CPU time
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
	parser[CFG_CONSTELLATION] = std::string("true");
	parser[CFG_DECODER_ADDRESS] = std::string(DEFAULT_DECODER_ADDRESS);
	parser[CFG_DECODER_PORT] = std::string(QUOTE(DEFAULT_DECODER_PORT));
	parser.SaveFile();
}

int main(int argc, char **argv) {
	ConfigParser parser("xritdemod.cfg");
	float rrcAlpha;
	uint32_t symbolRate, centerFrequency, sampleRate;
	uint8_t lnaGain = 15, vgaGain = 15, mixerGain = 15;
	bool agcEnable = false;
	bool constellationEnable = true;
	float pllAlpha = (float)CLOCK_ALPHA;
	std::string decoderAddress(DEFAULT_DECODER_ADDRESS);
	int decoderPort = DEFAULT_DECODER_PORT;
	int deviceNumber = DEFAULT_DEVICE_NUMBER;

	try {

		std::cout << "xRIT Demodulator - v" << QUOTE(MAJOR_VERSION) << "." << QUOTE(MINOR_VERSION) << "." << QUOTE(MAINT_VERSION) << " -- " << QUOTE(GIT_SHA1) << std::endl;
	#ifdef NON_FREE
		std::cout << "  Compiled with NON FREE support." << std::endl;
	#endif
		std::cout << "  Compilation Date/Time: " << __DATE__ << " - " << __TIME__ << std::endl;
		std::cout << "  SatHelper Version: " << SatHelper::Info::GetVersion() << " - " << SatHelper::Info::GetGitSHA1() << std::endl;
		std::cout << "  SatHelper Compilation Date/Time: " << SatHelper::Info::GetCompilationDate() << " - " << SatHelper::Info::GetCompilationTime() << std::endl;
		std::cout << std::endl;

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

		if (parser.hasKey(CFG_PLL_ALPHA)) {
			std::cout << "Warning: PLL Alpha is not the default one. Use with care." << std::endl;
			pllAlpha = parser.getFloat(CFG_PLL_ALPHA);
		}

		if (parser.hasKey(CFG_CONSTELLATION)) {
			constellationEnable = parser.getBool(CFG_CONSTELLATION);
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

		if (parser.hasKey(CFG_DEVICE_NUM)) {
			deviceNumber = parser.getInt(CFG_DEVICE_NUM);
		}

		if (parser.hasKey(CFG_DECODER_ADDRESS)) {
			decoderAddress = parser.get(CFG_DECODER_ADDRESS);
		}

		if (parser.hasKey(CFG_DECODER_PORT)) {
			decoderPort = parser.getInt(CFG_DECODER_PORT);
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
				std::cout << "Airspy Frontend selected." << std::endl;
				AirspyDevice::Initialize();
				try {
					device = new AirspyDevice();
				} catch (SatHelperException &e) {
					std::cerr << "Failed to open Airspy Device: " << e.reason() << std::endl;
					exit(1);
				}
				bool sampleRateSet = false;

				std::vector<uint32_t> sampleRates = device->GetAvailableSampleRates();
				for (uint32_t asSR : sampleRates) {
					if (asSR == sampleRate) {
						device->SetSampleRate(sampleRate);
						sampleRateSet = true;
						break;
					}
				}

				if (!sampleRateSet) {
					std::cerr << "Your device is not compatible with sampleRate \"" << sampleRate << "\"" << std::endl;
					return 1;
				}

				std::cout << "Airspy sample rate set to " << device->GetSampleRate() << std::endl;

			} else if (parser[CFG_DEVICE_TYPE] == "cfile") {
				if (!parser.hasKey(CFG_FILENAME)) {
					std::cerr << "Device Type defined as \"cfile\" but no \"filename\" specified." << std::endl;
					return 1;
				}
				std::cout << "CFile Frontend selected. File Name: " << parser[CFG_FILENAME] << std::endl;
				device = new CFileFrontend(parser[CFG_FILENAME]);
				device->SetCenterFrequency(centerFrequency);
				device->SetSampleRate(sampleRate);
			} else if (parser[CFG_DEVICE_TYPE] == "wav") {
				std::cerr << "WAV Reader not implemented." << std::endl;
				return 1;
			} else if (parser[CFG_DEVICE_TYPE] == "rtlsdr") {
				std::cout << "RTLSDR Frontend selected. Device Number: " << deviceNumber << std::endl;
				device = new RtlFrontend(deviceNumber);
				device->SetSampleRate(sampleRate);
				device->SetCenterFrequency(centerFrequency);
			} else if (parser[CFG_DEVICE_TYPE] == "spyserver") {
				std::cout << "SpyServer Frontend selected." << std::endl;
				if (!parser.hasKey(CFG_SPYSERVER_HOST)) {
					std::cerr << "No Host specified with tag " CFG_SPYSERVER_HOST << std::endl;
					return 1;
				}

				if (!parser.hasKey(CFG_SPYSERVER_PORT)) {
					std::cerr << "No Port specified with tag " CFG_SPYSERVER_PORT << std::endl;
					return 1;
				}
				device = new SpyServerFrontend(parser[CFG_SPYSERVER_HOST], parser.getInt(CFG_SPYSERVER_PORT));
				SpyServerFrontend *d = (SpyServerFrontend *)(device);
				d->Connect();
				std::cout << "Server Device: " << d->GetName() << std::endl;
				device->SetCenterFrequency(centerFrequency);
				device->SetSampleRate(sampleRate);
#if 0
			} else if (parser[CFG_DEVICE_TYPE] == "hackrf") {
				std::cout << "HackRF Frontend selected. Device Number: " << deviceNumber << std::endl;
				HackRFFrontend::Initialize();
				device = new HackRFFrontend(deviceNumber);
				device->SetSampleRate(sampleRate);
				device->SetCenterFrequency(centerFrequency);
#endif
	#ifdef NON_FREE
			} else if (parser[CFG_DEVICE_TYPE] == "sdrplay") {
				std::cout << "SDRPlay Frontend selected." << std::endl;
				SDRPlayFrontend::Initialize();
				device = new SDRPlayFrontend();
				device->SetSampleRate(sampleRate);
				device->SetCenterFrequency(centerFrequency);
			}
	#else
			}
	#endif
		} else {
			std::cerr << "Input Device Type not specified in config file." << std::endl;
			return 1;
		}

		if (constellationEnable) {
			diagManager = new DiagManager(0.01f);
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
		costasLoop = new CostasLoop(pllAlpha, LOOP_ORDER);
		clockRecovery = new ClockRecovery(sps, CLOCK_GAIN_OMEGA, CLOCK_MU, CLOCK_ALPHA, CLOCK_OMEGA_LIMIT);
		rrcFilter = new FirFilter(1, rrcTaps);
		symbolManager = new SymbolManager(decoderAddress, decoderPort);

		std::cout << "Center Frequency: " << (centerFrequency / 1000000.0) << " MHz" << std::endl;
		std::cout << "Automatic Gain Control: " << (agcEnable ? "Enabled" : "Disabled") << std::endl;
		if (!agcEnable) {
			std::cout << "	LNA Gain: " << (int)lnaGain << std::endl;
			std::cout << "	VGA Gain: " << (int)vgaGain << std::endl;
			std::cout << "	MIX Gain: " << (int)mixerGain << std::endl;
		}

		device->SetCenterFrequency(centerFrequency);
		if (agcEnable) {
			device->SetAGC(true);
		} else {
			device->SetAGC(false);
			device->SetLNAGain(lnaGain);
			device->SetLNAGain(vgaGain);
			device->SetMixerGain(mixerGain);
		}

		std::cout << "Starting " << device->GetName() << std::endl;
		device->Start();
		running = true;

		std::thread symbolThread(&symbolLoopFunc);

		ExitHandler::setCallback([](int signal) {
			std::cout << std::endl << "Got Ctrl + C! Closing..." << std::endl;
			running = false;
		});

		ExitHandler::registerSignal();

		while (running) {
			int siq = symbolManager->symbolsInQueue();
			if (siq > 0) {
				symbolManager->process();
			}
			std::this_thread::sleep_for(std::chrono::microseconds(10)); // Let's not waste CPU time
		}

		std::cout << "Stopping " << device->GetName() << std::endl;
		device->Stop();

		std::cout << "Stopping Symbol Processing Thread" << std::endl;
		symbolThread.join();

		std::cout << "Closing..." << std::endl;
	} catch (SatHelperException &e) {
		std::cerr << "Unhandled exception: " << e.reason() << std::endl;
	}

	if (agc != NULL) {
		delete agc;
	}
	if (costasLoop != NULL) {
		delete costasLoop;
	}
	if (clockRecovery != NULL) {
		delete clockRecovery;
	}
	if (rrcFilter != NULL) {
		delete rrcFilter;
	}
	if (device != NULL) {
		delete device;
	}
	if (symbolManager != NULL) {
		delete symbolManager;
	}
	if (decimator != NULL) {
		delete decimator;
	}
	if (diagManager != NULL) {
		delete diagManager;
	}
	if (buffer0 != NULL) {
		delete[] buffer0;
	}
	if (buffer1 != NULL) {
		delete[] buffer1;
	}

	return 0;
}

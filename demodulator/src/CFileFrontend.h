/*
 * CFileFrontend.h
 *
 *  Created on: 31/01/2017
 *      Author: Lucas Teske
 */

#ifndef SRC_CFILEFRONTEND_H_
#define SRC_CFILEFRONTEND_H_

#include "FrontendDevice.h"
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>
#include <complex>

namespace SatHelper {

class CFileFrontend: public FrontendDevice {
private:
	std::function<void(void *data, int length, int type)> cb;
	std::thread *mainThread;
	std::chrono::time_point<std::chrono::high_resolution_clock> t0;
	std::string filename;

    std::atomic_bool running;
	void threadLoop();
	uint32_t sampleRate;
	uint32_t centerFrequency;

	std::vector<std::complex<float>> sampleBuffer;
	std::complex<float> *sampleBufferPtr;

	const static std::string frontendName;
	const static std::vector<uint32_t> availableSampleRates;

public:
	CFileFrontend(const std::string &filename);
	virtual ~CFileFrontend();

	uint32_t SetSampleRate(uint32_t sampleRate) override;
	uint32_t SetCenterFrequency(uint32_t centerFrequency) override;
	const std::vector<uint32_t>& GetAvailableSampleRates() override;
	void Start() override;
	void Stop() override;
	void SetAGC(bool agc) override;

	void SetLNAGain(uint8_t value) override;
	void SetVGAGain(uint8_t value) override;
	void SetMixerGain(uint8_t value) override;
	uint32_t GetCenterFrequency() override;
	const std::string &GetName() override;
	uint32_t GetSampleRate() override;
	void SetSamplesAvailableCallback(std::function<void(void*data, int length, int type)> cb) override;
};

} /* namespace SatHelper */

#endif /* SRC_CFILEFRONTEND_H_ */

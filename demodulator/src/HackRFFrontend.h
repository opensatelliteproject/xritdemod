/*
 * HackRFFrontend.h
 *
 *  Created on: 26/02/2017
 *      Author: Lucas Teske
 */

#ifndef SRC_HACKRFFRONTEND_H_
#define SRC_HACKRFFRONTEND_H_

#include "FrontendDevice.h"
#include <libhackrf/hackrf.h>

class HackRFFrontend: public FrontendDevice {
private:
	std::function<void(void*data, int length, int type)> cb;
	hackrf_device *device;
	int deviceNumber;
	static const std::string FrontendName;
	static const std::vector<uint32_t> supportedSampleRates;

	static int callback(hackrf_transfer* transfer);

	uint8_t lnaGain;
	uint8_t vgaGain;
	uint8_t mixerGain;

	uint32_t centerFrequency;
	uint32_t sampleRate;
	float *buffer;
	float lut[256];
	int bufferLength;
	float alpha;
	float iavg;
	float qavg;
public:
	static void Initialize();
	static void DeInitialize();

	HackRFFrontend(int deviceNum);
	virtual ~HackRFFrontend();

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

#endif /* SRC_HACKRFFRONTEND_H_ */

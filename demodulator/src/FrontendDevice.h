/*
 * FrontendDevice.h
 *
 *  Created on: 31/01/2017
 *      Author: Lucas Teske
 */

#ifndef SRC_FRONTENDDEVICE_H_
#define SRC_FRONTENDDEVICE_H_

#define FRONTEND_SAMPLETYPE_FLOATIQ 0

#include <cstdint>
#include <functional>
#include <vector>

class FrontendDevice {

public:
	virtual ~FrontendDevice() {}
	virtual uint32_t SetSampleRate(uint32_t sampleRate) = 0;
	virtual uint32_t SetCenterFrequency(uint32_t centerFrequency) = 0;
	virtual const std::vector<uint32_t>& GetAvailableSampleRates() = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void SetAGC(bool agc) = 0;

	virtual void SetLNAGain(uint8_t value) = 0;
	virtual void SetVGAGain(uint8_t value) = 0;
	virtual void SetMixerGain(uint8_t value) = 0;
	virtual uint32_t GetCenterFrequency() = 0;
	virtual const std::string &GetName() = 0;
	virtual uint32_t GetSampleRate() = 0;
	virtual void SetSamplesAvailableCallback(std::function<void(void*data, int length, int type)> cb) = 0;
};

#endif /* SRC_FRONTENDDEVICE_H_ */

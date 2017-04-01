/*
 * AirspyDevice.h
 *
 *  Created on: 24/12/2016
 *      Author: Lucas Teske
 */

#ifndef SRC_AIRSPYDEVICE_H_
#define SRC_AIRSPYDEVICE_H_

#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <functional>
#include "FrontendDevice.h"

extern "C" {
#include <libairspy/airspy.h>
}

namespace OpenSatelliteProject {

class AirspyDevice: public FrontendDevice {
private:
	static std::string libraryVersion;

	std::function<void(void *data, int length, int type)> cb;
	uint8_t boardId;
	std::string firmwareVersion;
	std::string partNumber;
	std::string serialNumber;
	std::vector<uint32_t> availableSampleRates;
	std::string name;
	airspy_device* device;

	uint32_t sampleRate;
	uint32_t centerFrequency;
	uint8_t lnaGain;
	uint8_t vgaGain;
	uint8_t mixerGain;

	int SamplesAvailableCallback(airspy_transfer *transfer);
public:
	AirspyDevice();
	virtual ~AirspyDevice();

	static void Initialize();
	static void DeInitialize();

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

} /* namespace OpenSatelliteProject */

#endif /* SRC_AIRSPYDEVICE_H_ */

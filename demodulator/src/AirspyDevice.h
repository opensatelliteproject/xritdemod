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

extern "C" {
#include <libairspy/airspy.h>
}

namespace OpenSatelliteProject {

class AirspyDevice {
private:
	static std::string libraryVersion;

	std::function<void(void *data, int length)> cb;
	uint8_t boardId;
	std::string firmwareVersion;
	std::string partNumber;
	std::string serialNumber;
	std::vector<uint32_t> availableSampleRates;
	std::string name;
	airspy_device* device;

	uint32_t sampleRate;
	uint32_t centerFrequency;

	int SamplesAvailableCallback(airspy_transfer *transfer);
public:
	AirspyDevice();
	virtual ~AirspyDevice();

	static void Initialize();
	static void DeInitialize();

	uint32_t SetSampleRate(uint32_t sampleRate);
	uint32_t SetCenterFrequency(uint32_t centerFrequency);
	const std::vector<uint32_t>& GetAvailableSampleRates();
	void Start();
	void Stop();
	void SetAGC(bool agc);

	void SetLNAGain(uint8_t value);
	void SetVGAGain(uint8_t value);
	void SetMixerGain(uint8_t value);

	inline uint32_t GetCenterFrequency() {
		return centerFrequency;
	}

	inline const std::string &GetName() {
		return name;
	}

	inline uint32_t GetSampleRate() {
		return sampleRate;
	}

	inline void SetSamplesAvailableCallback(std::function<void(void*data, int length)> cb) {
		this->cb = cb;
	}

};

} /* namespace OpenSatelliteProject */

#endif /* SRC_AIRSPYDEVICE_H_ */

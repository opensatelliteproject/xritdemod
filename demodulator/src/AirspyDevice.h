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
	std::vector<uint64_t> availableSampleRates;
	std::string name;
	airspy_device* device;

	uint64_t sampleRate;
	uint64_t centerFrequency;

	int SamplesAvailableCallback(airspy_transfer *transfer);
public:
	AirspyDevice();
	virtual ~AirspyDevice();

	static void Initialize();
	static void DeInitialize();

	uint64_t SetSampleRate(uint64_t sampleRate);
	uint64_t SetCenterFrequency(uint64_t centerFrequency);
	const std::vector<uint64_t>& GetAvailableSampleRates();
	void Start();
	void Stop();
	void SetAGC(bool agc);

	void SetLNAGain(uint8_t value);
	void SetVGAGain(uint8_t value);
	void SetMixerGain(uint8_t value);

	inline uint64_t GetCenterFrequency() {
		return centerFrequency;
	}

	inline const std::string &GetName() {
		return name;
	}

	inline uint64_t GetSampleRate() {
		return sampleRate;
	}

	inline void SetSamplesAvailableCallback(std::function<void(void*data, int length)> cb) {
		this->cb = cb;
	}

};

} /* namespace OpenSatelliteProject */

#endif /* SRC_AIRSPYDEVICE_H_ */

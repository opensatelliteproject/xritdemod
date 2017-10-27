/*
 * SDRPlayFrontend.h
 *
 *  Created on: 27/02/2017
 *      Author: Lucas Teske
 */

#ifdef NON_FREE
// Since SDRPlay API is a closed source one (only binary release), you need to explicit enable NON_FREE .

#ifndef SDRPLAYFRONTEND_H_
#define SDRPLAYFRONTEND_H_

#include "FrontendDevice.h"

class SDRPlayFrontend: public FrontendDevice {
private:

	static void internalCallback(short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, void *cbContext);
	static void internalCallbackGC(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext);
	static const std::string FrontendName;
	static const std::vector<uint32_t> supportedSampleRates;
	float *buffer;
	unsigned int bufferLength;

	std::function<void(void *data, int length, int type)> cb;

	uint32_t sampleRate;
	uint32_t centerFrequency;

	int gRdB;
	int gRdBsystem;
	int samplesPerPacket;
public:

	static void Initialize();
	static void DeInitialize();

	SDRPlayFrontend();
	virtual ~SDRPlayFrontend();

	uint32_t SetSampleRate(uint32_t sampleRate) override;
	uint32_t SetCenterFrequency(uint32_t centerFrequency) override;
	const std::vector<uint32_t>& GetAvailableSampleRates() override;
	void Start() override;
	void Stop() override;
	void SetAGC(bool agc) override;

	void SetAntenna(int antenna);

	void SetLNAGain(uint8_t value) override;
	void SetVGAGain(uint8_t value) override;
	void SetMixerGain(uint8_t value) override;
	void SetBiasT(uint8_t value) override;

	uint32_t GetCenterFrequency() override;

	const std::string &GetName() override;

	uint32_t GetSampleRate() override;

	void SetSamplesAvailableCallback(std::function<void(void*data, int length, int type)> cb) override;
};

#endif /* SDRPLAYFRONTEND_H_ */

#endif

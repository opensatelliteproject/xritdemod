/*
 * SampleFIFO.cpp
 *
 *  Created on: 06/01/2017
 *      Author: Lucas Teske
 */

#include "SampleFIFO.h"

namespace SatHelper {

SampleFIFO::~SampleFIFO() {
}

void SampleFIFO::addSamples(float *data, int length) {
	fifoMutex.lock();
	for (int i = 0; i < length; i++) {
		if (samples.size() + 1 > maxLength) {
			overflow = true;
			samples.pop();
		} else {
			overflow = false;
		}
		samples.push(data[i]);
	}
	fifoMutex.unlock();
}

void SampleFIFO::addSample(float data) {
	fifoMutex.lock();

	if (samples.size() + 1 > maxLength) {
		overflow = true;
		samples.pop();
	} else {
		overflow = false;
	}

	samples.push(data);
	fifoMutex.unlock();
}

float SampleFIFO::takeSample() {
	float v = 0.0f;

	fifoMutex.lock();
	if (samples.size() > 0) {
		v = samples.front();
		samples.pop();
		overflow = false;
	}
	fifoMutex.unlock();

	return v;
}

unsigned int SampleFIFO::size() {
	unsigned int size;

	fifoMutex.lock();
	size = samples.size();
	fifoMutex.unlock();

	return size;
}

bool SampleFIFO::containsSamples() {
	bool ret;

	fifoMutex.lock();
	ret = samples.size() > 0;
	fifoMutex.unlock();

	return ret;
}

} /* namespace SatHelper */

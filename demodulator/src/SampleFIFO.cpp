/*
 * SampleFIFO.cpp
 *
 *  Created on: 06/01/2017
 *      Author: Lucas Teske
 */

#include "SampleFIFO.h"

namespace SatHelper {

SampleFIFO::~SampleFIFO() {
	delete[] baseBuffer;
}

void SampleFIFO::addSamples(const float *data, unsigned int length) {
	fifoMutex.lock();
	bool trimmed = false;
	if (length >= maxLength) {
		overflow = true;
		length = maxLength;
		trimmed = true;
	}

	if (numItems + length == maxLength) {
		curSample += length;
		curSample %= maxLength;
		numItems -= length;
		overflow = true;
	} else if (!trimmed) {
		overflow = false;
	}

	for (unsigned int i = 0; i < length; i++) {
		*getPositionPointer(curSample + numItems) = data[i];
		numItems++;
	}
	fifoMutex.unlock();
}

void SampleFIFO::addSamples(const int16_t *data, unsigned int length) {
	fifoMutex.lock();

	bool trimmed = false;
	if (length >= maxLength) {
		overflow = true;
		length = maxLength;
		trimmed = true;
	}

	if (numItems + length >= maxLength) {
		curSample += length;
		numItems -= length;
		overflow = true;
	} else if (!trimmed) {
		overflow = false;
	}

	for (unsigned int i = 0; i < length; i++) {
		*getPositionPointer(curSample + numItems) = ((float)data[i]) / 32768.0f;
		numItems++;
	}
	fifoMutex.unlock();
}

void SampleFIFO::addSample(float data) {
	fifoMutex.lock();

	if (numItems + 1 == maxLength) {
		curSample++;
		curSample %= maxLength;
		numItems--;
		overflow = true;
	} else {
		overflow = false;
	}

	*getPositionPointer(curSample + numItems) = data;
	numItems++;

	fifoMutex.unlock();
}

float SampleFIFO::takeSample() {
	float v = 0.0f;

	fifoMutex.lock();
	if (numItems > 0) {
		v = unsafe_takeSample();
	}
	fifoMutex.unlock();

	return v;
}

unsigned int SampleFIFO::size() {
	unsigned int size;

	fifoMutex.lock();
	size = numItems;
	fifoMutex.unlock();

	return size;
}

bool SampleFIFO::containsSamples() {
	bool ret;

	fifoMutex.lock();
	ret = numItems > 0;
	fifoMutex.unlock();

	return ret;
}

} /* namespace SatHelper */

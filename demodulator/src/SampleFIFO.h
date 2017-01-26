/*
 * SampleFIFO.h
 *
 *  Created on: 06/01/2017
 *      Author: Lucas Teske
 */

#ifndef SRC_SAMPLEFIFO_H_
#define SRC_SAMPLEFIFO_H_

#include <queue>
#include <mutex>
#include <atomic>

namespace SatHelper {

class SampleFIFO {
private:
	float *baseBuffer;
	int curSample;

	std::mutex fifoMutex;
	unsigned int maxLength;
	std::atomic_bool overflow;
	unsigned int numItems;

	inline float *getPositionPointer(int pos) {
		return &baseBuffer[pos % maxLength];
	}
public:
	SampleFIFO(const unsigned int maxLength) :
		 curSample(0), maxLength(maxLength), overflow(false), numItems(0) {
		baseBuffer = new float[maxLength];
	}

	virtual ~SampleFIFO();

	// Safe operations
	void addSamples(const float *data, unsigned int length);
	void addSamples(const int16_t *data, unsigned int length);
	void addSample(float data);
	float takeSample();
	unsigned int size();
	bool containsSamples();

	inline bool isOverflow() { return overflow; }

	// Unsafe operations
	inline void unsafe_lockMutex() {
		fifoMutex.lock();
	}

	inline void unsafe_unlockMutex() {
		fifoMutex.unlock();
	}

	inline float unsafe_takeSample() {
		float v = *getPositionPointer(curSample);
		curSample++;
		curSample %= maxLength;
		numItems-=1;
		overflow = false;
		return v;
	}
};

} /* namespace SatHelper */

#endif /* SRC_SAMPLEFIFO_H_ */

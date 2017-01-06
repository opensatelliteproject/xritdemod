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
	std::queue<float> samples;
	std::mutex fifoMutex;
	unsigned int maxLength;
	std::atomic_bool overflow;

public:
	SampleFIFO(unsigned int maxLength) :
			maxLength(maxLength), overflow(false) {
	}

	virtual ~SampleFIFO();

	// Safe operations
	void addSamples(float *data, int length);
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
		float v = samples.front();
		samples.pop();
		overflow = false;
		return v;
	}
};

} /* namespace SatHelper */

#endif /* SRC_SAMPLEFIFO_H_ */

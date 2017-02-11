/*
 * DiagManager.h
 *
 *  Created on: 07/02/2017
 *      Author: Lucas Teske
 */

#ifndef SRC_DIAGMANAGER_H_
#define SRC_DIAGMANAGER_H_

#include <SatHelper/sathelper.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>
#include <complex>

#define DM_BUFFER_SIZE 2048

class DiagManager {
private:
	SatHelper::CircularBuffer<float> buffer;
	void threadLoop();

	std::thread *mainThread;
	std::chrono::time_point<std::chrono::high_resolution_clock> t0;
	std::atomic_bool running;
	SatHelper::UdpSocket socket;
	float interval;

public:
	DiagManager(float interval);
	virtual ~DiagManager();

	void addSamples(const float *data, int length);
};

#endif /* SRC_DIAGMANAGER_H_ */

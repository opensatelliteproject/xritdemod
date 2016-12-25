/*
 * SymbolManager.cpp
 *
 *  Created on: 25/12/2016
 *      Author: Lucas Teske
 */

#include "SymbolManager.h"
#include <chrono>
#include <iostream>
#include <algorithm>

#define RECONNECT_DELAY 1000

namespace OpenSatelliteProject {

SymbolManager::SymbolManager() :
		client(std::string("127.0.0.1"), 5000) {
	running = true;
	dataThread = new std::thread(
			std::bind(&SymbolManager::dataThreadLoop, this));
	buffer = new char[SM_SOCKET_BUFFER_SIZE];
	inBufferLength = 0;
}

SymbolManager::~SymbolManager() {
	delete buffer;
}

void SymbolManager::dataThreadLoop() {
	std::chrono::milliseconds timespan(RECONNECT_DELAY);
	bool isConnected = false;

	while (running) {

		std::cout << "Trying to connect to decoder at port 5000" << std::endl;
		try {
			client.Connect();
			isConnected = true;
		} catch (SatHelper::SocketConnectException &e) {
			isConnected = false;
			std::cerr << "Error connecting to localhost:5000: " << e.reason()
					<< std::endl;
		}

		while (isConnected) {

			dataMutex.lock();
			if (dataQueue.size() > 0) {
				int copySize =
						dataQueue.size() > SM_SOCKET_BUFFER_SIZE ?
								SM_SOCKET_BUFFER_SIZE : dataQueue.size();
				for (int i = 0; i < copySize; i++) {
					float f = dataQueue.front() * 127;
					f = f > 127 ? 127 : f;
					f = f < -128 ? -128 : f;
					buffer[i] = (char) (f);
					dataQueue.pop();
				}

				inBufferLength = copySize;
			}
			dataMutex.unlock();

			if (inBufferLength > 0) {
				try {
					client.Send(buffer, inBufferLength);
				} catch (SatHelper::ClientDisconnectedException &c) {
					std::cout << "Disconnected from decoder.\n";
					isConnected = false;
					client.Close();
				} catch (SatHelper::SocketException &e) {
					std::cout << "There was an exception for a client: "
							<< e.reason() << std::endl;
					isConnected = false;
					client.Close();
				}
				inBufferLength = 0;
			}
		}

		if (!isConnected) {
			// Clear DataQueue
			dataMutex.lock();
			int length = dataQueue.size();
			for (int i = 0; i < length; i++) {
				dataQueue.pop();
			}
			dataMutex.unlock();
		}
		std::this_thread::sleep_for(timespan);
	}
}

void SymbolManager::add(float *data, int length) {
	dataMutex.lock();
	for (int i = 0; i < length; i++) {
		dataQueue.push(data[i]);
	}
	dataMutex.unlock();
}

void SymbolManager::add(std::complex<float> *data, int length) {
	dataMutex.lock();
	for (int i = 0; i < length; i++) {
		dataQueue.push(data[i].real());
	}
	dataMutex.unlock();
}

} /* namespace OpenSatelliteProject */

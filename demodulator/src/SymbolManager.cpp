/*
 * SymbolManager.cpp
 *
 *  Created on: 25/12/2016
 *      Author: Lucas Teske
 */

#include "SymbolManager.h"

namespace OpenSatelliteProject {

SymbolManager::SymbolManager(std::string &address, int port) :
		client(address, port) {
	buffer = new char[SM_SOCKET_BUFFER_SIZE];
	inBufferLength = 0;
	isConnected = false;
}

SymbolManager::~SymbolManager() {
	delete[] buffer;
}

void SymbolManager::process() {
	if (!isConnected) {
		std::cout << "Trying to connect to decoder at " << client.GetAddress().ToString() << ":" << client.GetPort() << std::endl;
		try {
			client.Connect();
			isConnected = true;
		} catch (SatHelper::SocketConnectException &e) {
			isConnected = false;
			std::cerr << "Error connecting to " << client.GetAddress().ToString() << ":" << client.GetPort() << " : " << e.reason() << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	if (isConnected) {
		dataMutex.lock();
		if (dataQueue.size() > 0) {
			size_t copySize =
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
			bool closeClient = false;
			try {
				client.Send(buffer, static_cast<int>(inBufferLength));
			} catch (SatHelper::ClientDisconnectedException &) {
				std::cout << "Disconnected from decoder.\n";
				isConnected = false;
				closeClient = true;
			} catch (SatHelper::SocketException &e) {
				std::cout << "There was an exception for a client: "
						<< e.reason() << std::endl;
				isConnected = false;
				closeClient = true;
			}
			inBufferLength = 0;

			if (closeClient) {
				try {
					client.Close();
				} catch (std::exception &e) {
					std::cerr << "Error closing client: " << e.what() << std::endl;
				}
			}
		}
	} else {
		// Clear DataQueue, we don't have any connected client.
		dataMutex.lock();
		dataQueue = std::queue<float>();
		dataMutex.unlock();
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

    if (dataQueue.size() >= SM_MAX_SYMBOL_BUFFER) {
        std::cerr << "SymbolManager Buffer is full!!! Dropping samples." << std::endl;
        dataMutex.unlock();
        return;
    }

	for (int i = 0; i < length; i++) {
		dataQueue.push(data[i].real()); // M&M Outputs the data in real (old was imaginary due bug in libSatHelper costas loop)
	}
	dataMutex.unlock();
}

} /* namespace OpenSatelliteProject */

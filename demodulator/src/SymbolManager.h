/*
 * SymbolManager.h
 *
 *  Created on: 25/12/2016
 *      Author: Lucas Teske
 */

#ifndef SRC_SYMBOLMANAGER_H_
#define SRC_SYMBOLMANAGER_H_

#include <mutex>
#include <thread>
#include <queue>
#include <complex>
#include <atomic>
#include <SatHelper/sathelper.h>

namespace OpenSatelliteProject {

// Number of symbols
#define SM_SOCKET_BUFFER_SIZE 16384
#define SM_MAX_SYMBOL_BUFFER (1024 * 1024)

class SymbolManager {
private:
    std::queue<float> dataQueue;
    std::mutex dataMutex;
    SatHelper::TcpClient client;
    char *buffer;
    size_t inBufferLength;
    bool isConnected;

public:
	SymbolManager(std::string &address, int port);
	virtual ~SymbolManager();
    void add(float *data, int length);
    void add(std::complex<float> *data, int length);
    void process();

    inline int symbolsInQueue() {
    	int s;
    	dataMutex.lock();
    	s = static_cast<int>(dataQueue.size());
    	dataMutex.unlock();
    	return s;
    }
};

} /* namespace OpenSatelliteProject */

#endif /* SRC_SYMBOLMANAGER_H_ */

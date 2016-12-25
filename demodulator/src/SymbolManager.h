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
#include <sathelper.h>

namespace OpenSatelliteProject {

// Number of symbols
#define SM_SOCKET_BUFFER_SIZE 1024

class SymbolManager {
private:
    std::queue<float> dataQueue;
    std::mutex dataMutex;
    std::thread *dataThread;

    std::atomic_bool running;
    void dataThreadLoop();
    SatHelper::TcpClient client;
    char *buffer;
    int inBufferLength;

public:
	SymbolManager();
	virtual ~SymbolManager();
    void add(float *data, int length);
    void add(std::complex<float> *data, int length);
};

} /* namespace OpenSatelliteProject */

#endif /* SRC_SYMBOLMANAGER_H_ */

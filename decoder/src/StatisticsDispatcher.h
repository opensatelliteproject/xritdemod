/*
 * StatisticsDispatcher.h
 *
 *  Created on: 01/12/2016
 *      Author: lucas
 */

#ifndef STATISTICSDISPATCHER_H_
#define STATISTICSDISPATCHER_H_

#include <sathelper.h>
#include "Statistics.h"
#include <thread>
#include <mutex>
#include <atomic>

class StatisticsDispatcher {
private:
    Statistics statistics;
    std::mutex dataMutex;
    std::thread *dataThread;
    std::vector<SatHelper::TcpSocket> clients;

    std::atomic_bool running;
    void dataThreadLoop();
    SatHelper::TcpServer server;
public:
    StatisticsDispatcher();
    ~StatisticsDispatcher();

    inline void Update(const Statistics &statistics) {
        dataMutex.lock();
        this->statistics.update(statistics);
        dataMutex.unlock();
    }
};

#endif /* STATISTICSDISPATCHER_H_ */

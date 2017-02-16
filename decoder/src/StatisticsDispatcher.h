/*
 * StatisticsDispatcher.h
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#ifndef STATISTICSDISPATCHER_H_
#define STATISTICSDISPATCHER_H_

#include <SatHelper/sathelper.h>
#include "Statistics.h"
#include <thread>
#include <mutex>
#include <atomic>

class StatisticsDispatcher {
private:
    Statistics statistics;
    std::vector<SatHelper::TcpSocket> clients;
    SatHelper::TcpServer server;
    int port;

public:
    StatisticsDispatcher(int port);
    ~StatisticsDispatcher();

    inline void Update(const Statistics &statistics) {
        this->statistics.update(statistics);
    }

    void Start();
    void Stop();
    void Work();
};

#endif /* STATISTICSDISPATCHER_H_ */

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
    std::vector<SatHelper::TcpSocket> clients;

    void dataThreadLoop();
    SatHelper::TcpServer server;
public:
    StatisticsDispatcher();
    ~StatisticsDispatcher();

    inline void Update(const Statistics &statistics) {
        this->statistics.update(statistics);
    }

    void Work();
};

#endif /* STATISTICSDISPATCHER_H_ */

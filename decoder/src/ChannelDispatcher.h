/*
 * ChannelDispatcher.h
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#ifndef CHANNELDISPATCHER_H_
#define CHANNELDISPATCHER_H_

#include <mutex>
#include <thread>
#include <queue>
#include <atomic>
#include <sathelper.h>
#include "ChannelPacket.h"
#include "Statistics.h"

class ChannelDispatcher {
private:
    std::queue<ChannelPacket *> dataQueue;
    std::mutex dataMutex;
    std::thread *dataThread;
    std::vector<SatHelper::TcpSocket> clients;

    std::atomic_bool running;
    void dataThreadLoop();
    SatHelper::TcpServer server;

public:
    ChannelDispatcher();
    virtual ~ChannelDispatcher();

    void add(char *data, int length);
};

#endif /* CHANNELDISPATCHER_H_ */

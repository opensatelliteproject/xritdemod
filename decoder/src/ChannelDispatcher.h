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
#include <SatHelper/sathelper.h>
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
    int port;
public:
    ChannelDispatcher(int port);
    virtual ~ChannelDispatcher();

    void add(char *data, int length);
    void Stop();
};

#endif /* CHANNELDISPATCHER_H_ */

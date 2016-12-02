/*
 * StatisticsDispatcher.cpp
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#include "StatisticsDispatcher.h"
#include <chrono>
#include <iostream>
#include <algorithm>

#define LOOP_DELAY 100

StatisticsDispatcher::StatisticsDispatcher() {
    running = true;
    dataThread = new std::thread(std::bind(&StatisticsDispatcher::dataThreadLoop, this));
}

StatisticsDispatcher::~StatisticsDispatcher() {
    running = false;
    dataThread->join();
    delete dataThread;
}

void StatisticsDispatcher::dataThreadLoop() {
    std::chrono::milliseconds timespan(LOOP_DELAY);
    std::vector<SatHelper::TcpSocket> toRemove;

    std::cout << "Starting Statistics Dispatcher at port 5002" << std::endl;
    server.Listen(5002, true);

    while (running) {
        // Check for new clients
        try {
            SatHelper::TcpSocket newClient = server.Accept();
            clients.push_back(newClient);
        } catch (SatHelper::SocketAcceptException &e) {
            // No new client.
        }

        // Process the data
        dataMutex.lock();
        if (clients.size() > 0) {
            for (SatHelper::TcpSocket &client: clients) {
                try {
                    std::cout << "sending statistics" << std::endl;
                    client.Send((char *)&statistics.GetData(), sizeof(Statistics_st));
                    std::cout << "statistics sent" << std::endl;
                } catch (SatHelper::ClientDisconnectedException &c) {
                    std::cout << "One client has been disconnected." << std::endl;
                    toRemove.push_back(client);
                    client.Close();
                } catch (SatHelper::SocketException &e) {
                    std::cout << "There was an exception for a client: " << e.reason() << std::endl;
                    toRemove.push_back(client);
                    client.Close();
                } catch (SatHelper::NotAllDataSentException &e) {
                    std::cout << "Not all data sent" << std::endl;
                    toRemove.push_back(client);
                    client.Close();
                } catch (SatHelper::SocketWriteException &e) {
                    std::cout << "Socket Write Exception" << std:: endl;
                    toRemove.push_back(client);
                    client.Close();
                }
            }
        }
        dataMutex.unlock();

        // Process dropped clients
        for(SatHelper::TcpSocket &s: toRemove) {
            clients.erase(
                    std::remove_if(
                            clients.begin(),
                            clients.end(),
                            [&s](SatHelper::TcpSocket &x) { return x.GetSocketFD() == s.GetSocketFD(); }
                    ),
                    clients.end()
         );
        }
        toRemove.clear();
        std::this_thread::sleep_for(timespan);
    }
}

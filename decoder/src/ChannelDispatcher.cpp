/*
 * ChannelDispatcher.cpp
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#include "ChannelDispatcher.h"
#include <chrono>
#include <iostream>
#include <algorithm>

#define LOOP_DELAY 1

ChannelDispatcher::ChannelDispatcher(int port) : running(true), port(port) {
    dataThread = new std::thread(&ChannelDispatcher::dataThreadLoop, this);
}

ChannelDispatcher::~ChannelDispatcher() {
    delete dataThread;
}

void ChannelDispatcher::Stop() {
    std::cout << "Closing channel dispatcher" << std::endl;
    running = false;
    if (dataThread->joinable()) {
        dataThread->join();
    }
    std:: cout << "Finished channel dispatcher" << std::endl;
}

void ChannelDispatcher::add(char *data, int length) {
    dataMutex.lock();
    dataQueue.push(new ChannelPacket(data, length));
    dataMutex.unlock();
}

void ChannelDispatcher::dataThreadLoop() {
    std::chrono::milliseconds timespan(LOOP_DELAY);
    std::vector<SatHelper::TcpSocket> toRemove;
    std::cout << "Starting Channel Dispatcher at port " << port << std::endl;
    server.Listen(port, true);

    while (running) {
        // Check for new clients
        try {
            SatHelper::TcpSocket newClient = server.Accept();
            clients.push_back(newClient);
        } catch (SatHelper::SocketAcceptException &) {
            // No new client.
        }

        // Process the data
        dataMutex.lock();
        if (dataQueue.size() > 0) {
            ChannelPacket *packet = dataQueue.front();
            if (clients.size() > 0) {
                for (SatHelper::TcpSocket &client : clients) {
                    try {
                        client.Send(packet->data, packet->length);
                    } catch (SatHelper::ClientDisconnectedException &) {
                        std::cout << "One client has been disconnected.\n";
                        toRemove.push_back(client);
                    } catch (SatHelper::SocketException &e) {
                        std::cout << "There was an exception for a client: " << e.reason() << std::endl;
                        toRemove.push_back(client);
                    }
                }
            }
            dataQueue.pop();
            delete packet;
        }
        dataMutex.unlock();

        // Process dropped clients
        for (SatHelper::TcpSocket &s : toRemove) {
            clients.erase(std::remove_if(clients.begin(), clients.end(), [&s](SatHelper::TcpSocket &x) {return x.GetSocketFD() == s.GetSocketFD();}),
                    clients.end());
        }

        toRemove.clear();
        if (!running) {
            break;
        }
        std::this_thread::sleep_for(timespan);
    }

    std::cout << "Closing Channel Dispatcher Server" << std::endl;
    for (SatHelper::TcpSocket &client : clients) {
        try {
            client.Close();
        } catch (SatHelper::SocketException &) {
            // Do Nothing
        }
    }
    server.Close();
}

/*
 * StatisticsDispatcher.cpp
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#include "StatisticsDispatcher.h"

#define LOOP_DELAY 100

StatisticsDispatcher::StatisticsDispatcher(int port) : port(port) {

}

void StatisticsDispatcher::Start() {
    std::cout << "Starting Statistics Dispatcher at port " << port << std::endl;
    server.Listen(port, true);
}

StatisticsDispatcher::~StatisticsDispatcher() {

}

void StatisticsDispatcher::Stop() {
    std::cout << "Disconnecting all statistics clients..." << std::endl;
    for (SatHelper::TcpSocket &client : clients) {
        try {
            client.Close();
        } catch (SatHelper::SocketException &) {
            // Do Nothing
        }
    }
    std::cout << "Closing statistics server..." << std::endl;
    server.Close();
    std::cout << "Statistics shutdown." << std::endl;
}

void StatisticsDispatcher::Work() {
    std::vector<SatHelper::TcpSocket> toRemove;
    // Check for new clients
    try {
        SatHelper::TcpSocket newClient = server.Accept();
        clients.push_back(newClient);
    } catch (SatHelper::SocketAcceptException &) {
        // No new client.
    }

    // Process the data
    if (clients.size() > 0) {
        for (SatHelper::TcpSocket &client: clients) {
            try {
                client.Send((char *)&statistics.GetData(), sizeof(Statistics_st));
            } catch (SatHelper::ClientDisconnectedException &) {
                std::cout << "One client has been disconnected.\n";
                toRemove.push_back(client);
                client.Close();
            } catch (SatHelper::NotAllDataSentException &) {
                std::cout << "Not all data sent\n";
                toRemove.push_back(client);
                client.Close();
            } catch (SatHelper::SocketWriteException &) {
                std::cout << "Socket Write Exception\n";
                toRemove.push_back(client);
                client.Close();
            } catch (SatHelper::SocketException &e) {
                std::cout << "There was an exception for a client: " << e.reason() << std::endl;
                toRemove.push_back(client);
                client.Close();
            }
        }
    }

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
}

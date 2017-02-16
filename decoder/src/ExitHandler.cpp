/*
 * ExitHandler.cpp
 *
 *  Created on: 15/02/2017
 *      Author: Lucas Teske
 */

#include "ExitHandler.h"
#include <iostream>

std::function<void(int type)> ExitHandler::callback = NULL;
#ifdef _WIN32
BOOL WINAPI ExitHandler::handler(DWORD sig) {
    if (sig == CTRL_C_EVENT || sig == CTRL_CLOSE_EVENT) {
        if (ExitHandler::callback != NULL) {
	        ExitHandler::callback(sig);
        }
        return true;
    }

    return false;
}
#else
void ExitHandler::handler(int sig) {
	if (ExitHandler::callback != NULL) {
		ExitHandler::callback(sig);
	}
}
#endif

void ExitHandler::setCallback(std::function<void(int type)> cb) {
	ExitHandler::callback = cb;
}

void ExitHandler::registerSignal() {
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(&ExitHandler::handler, TRUE)) {
        std::cerr << "ERROR: Could not set control handler" << std::endl;
    }
#else
	   struct sigaction sigIntHandler;

	   sigIntHandler.sa_handler = &ExitHandler::handler;
	   sigemptyset(&sigIntHandler.sa_mask);
	   sigIntHandler.sa_flags = 0;

	   sigaction(SIGINT, &sigIntHandler, NULL);
#endif
}

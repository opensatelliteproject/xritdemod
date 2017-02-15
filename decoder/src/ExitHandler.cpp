/*
 * ExitHandler.cpp
 *
 *  Created on: 15/02/2017
 *      Author: Lucas Teske
 */

#include "ExitHandler.h"

std::function<void(int type)> ExitHandler::callback = NULL;

void ExitHandler::handler(int sig) {
#ifdef _WIN32
	if (sig == CTRL_C_EVENT)
#endif
	if (ExitHandler::callback != NULL) {
		ExitHandler::callback(sig);
	}
#ifdef _WIN32
}
#endif
}

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

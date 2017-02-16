/*
* ExitHandler.h
*
*  Created on: 15/02/2017
*      Author: Lucas Teske
*/

#ifndef SRC_EXITHANDLER_H_
#define SRC_EXITHANDLER_H_

#include <functional>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

class ExitHandler {
private:
    static std::function<void(int type)> callback;
#ifdef _WIN32
    static BOOL WINAPI ExitHandler::handler(DWORD sig);
#else
    static void handler(int sig);
#endif
public:
	static void setCallback(std::function<void(int type)> cb);
	static void registerSignal();
};

#endif /* SRC_EXITHANDLER_H_ */

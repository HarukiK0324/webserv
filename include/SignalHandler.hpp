#ifndef SIGNALHANDLER_HPP
#define SIGNALHANDLER_HPP

#include "FdHandler.hpp"
#include "Utils.hpp"
#include <signal.h>
#include <sys/wait.h>
#include <cstdio>

class ServerLoop;

class SignalHandler : public FdHandler
{
private:
	int _sig;

public:
	SignalHandler(ServerLoop &loop);
	~SignalHandler();
	virtual void ReadEvent(Event ev);
};

#endif
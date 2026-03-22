#include "../include/FdHandler.hpp"

FdHandler::FdHandler(int fd, Event event, ServerLoop &loop) :
	_fd(fd), _event(event), _loop(loop)
{
}

FdHandler::~FdHandler()
{
	return;
}

int FdHandler ::GetFd() const
{
	return _fd;
}

Event FdHandler::GetEventMask()
{
	return _event;
}
void FdHandler::CheckTimeout(const time_t &_time)
{
	(void)_time;
}

void FdHandler ::SetFd(int fd)
{
	_fd = fd;
}

void FdHandler ::SetEvent(Event event)
{
	_event = event;
}

void FdHandler ::ReadEvent(Event ev)
{
	(void)ev;
}

void FdHandler ::WriteEvent(Event ev)
{
	(void)ev;
}
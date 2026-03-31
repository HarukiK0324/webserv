
#include "AFdHandler.hpp"

AFdhandler::AFdhandler(int fd, Event event, ServerLoop &loop) :
	_fd(fd), _event(event), _loop(loop)
{
}

AFdhandler::~AFdhandler()
{
	return;
}

int AFdhandler ::GetFd() const
{
	return _fd;
}

Event AFdhandler::GetEventMask()
{
	return _event;
}
void AFdhandler::CheckTimeout(const time_t &_time)
{
	(void)_time;
}

void AFdhandler ::SetFd(int fd)
{
	_fd = fd;
}

void AFdhandler ::SetEvent(Event event)
{
	_event = event;
}

void AFdhandler ::ReadEvent(Event ev)
{
	(void)ev;
}

void AFdhandler ::WriteEvent(Event ev)
{
	(void)ev;
}
#ifndef FDHANDLER_HPP
#define FDHANDLER_HPP

#include <unistd.h>
#include <ctime>
#include <poll.h>

typedef short Event;
class ServerLoop;

class FdHandler
{
protected:
	int _fd;
	Event _event;
	ServerLoop &_loop;

public:
	FdHandler(int fd, Event event, ServerLoop &loop);
	virtual ~FdHandler();
	int GetFd() const;
	Event GetEventMask();
	void SetFd(int fd);
	void SetEvent(Event event);
	virtual void CheckTimeout(const time_t &_time);
	virtual void ReadEvent(Event ev);
	virtual void WriteEvent(Event ev);
};

#endif
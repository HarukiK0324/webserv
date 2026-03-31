#ifndef AFDHANDLER_HPP
#define AFDHANDLER_HPP

#include <unistd.h>
#include <ctime>
#include <poll.h>

typedef short Event;
class ServerLoop;

class AFdhandler
{
protected:
	int _fd;
	Event _event;
	ServerLoop &_loop;

public:
	AFdhandler(int fd, Event event, ServerLoop &loop);
	virtual ~AFdhandler();
	int GetFd() const;
	Event GetEventMask();
	void SetFd(int fd);
	void SetEvent(Event event);
	virtual void CheckTimeout(const time_t &_time);
	virtual void ReadEvent(Event ev) = 0;
	virtual void WriteEvent(Event ev) = 0;
};

#endif
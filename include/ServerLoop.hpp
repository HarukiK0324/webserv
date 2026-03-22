#ifndef SERVERLOOP_HPP
#define SERVERLOOP_HPP

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <poll.h>
#include <ctime>
#include "FdHandler.hpp"
#include "Session.hpp"

#define EPOLL_LEN 50
#define SERVER_TIMEOUT 1000
extern int g_sig_pipe[2];

struct ServerConfig;
class SignalHandler;



class ServerLoop
{
private:
	std::map< int, std::map< std::string, ServerConfig > > _confs;	// fd  conf
	std::map< int, FdHandler * > _handlers;
	std::map< std::string, Session> _sessions;
	bool _run;
	void RunServer();
	void StopServer();
	friend class SignalHandler;	 //SIGINTの時、StopServer()を呼び出すため
public:
	ServerLoop(std::map< int, std::map< std::string, ServerConfig > > &confs);
	~ServerLoop();
	void RegisterHandler(FdHandler *h);
	void RemoveHandler(FdHandler *h);
	void ModifyHandler(FdHandler *h, Event new_event, int new_fd);
	bool hasSession(const std::string &session_id) const { return _sessions.find(session_id) != _sessions.end(); }
	Session &getSessions(const std::string &session_id) { return _sessions.find(session_id)->second; }
	void setSessions(const std::string &session_id, const Session &session) { _sessions[session_id] = session; }
	void removeSession(const std::string &session_id) { _sessions.erase(session_id); }
};

#endif
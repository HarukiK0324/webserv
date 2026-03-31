#include "ServerLoop.hpp"
#include "ListenHandler.hpp"
#include <sstream>
#include <sys/wait.h>

int server_run = 1;

ServerLoop::ServerLoop(std::map< int, std::map< std::string, ServerConfig > > &confs) :
	_confs(confs)
{
	for (std::map< int, std::map< std::string, ServerConfig > >::iterator it = _confs.begin(); it != _confs.end(); ++it)
	{
		ListenHandler *LHandler = new ListenHandler(it->first, *this, it->second);
		this->RegisterHandler(LHandler);
	}
	this->RunServer();
}

ServerLoop::~ServerLoop()
{
	std::cout << "end server" << std::endl;
	std::map< int, AFdhandler * >::iterator it = _handlers.begin();
	while (it != _handlers.end())
	{
		AFdhandler *h = it->second;
		std::map< int, AFdhandler * >::iterator to_erase = it++;
		_handlers.erase(to_erase);
		delete h;
	}
}

void ServerLoop::RunServer()
{
	while (server_run)
	{
		std::vector< struct pollfd > pfds;
		pfds.reserve(_handlers.size());
		time_t now = std::time(NULL);
		for (std::map< int, AFdhandler * >::iterator it = _handlers.begin(); it != _handlers.end(); ++it)
		{
			it->second->CheckTimeout(now);
			struct pollfd pfd;
			pfd.fd = it->first;
			pfd.events = it->second->GetEventMask();
			pfd.revents = 0;
			pfds.push_back(pfd);
		}
		int n = poll(&pfds[0], pfds.size(), SERVER_TIMEOUT);
		if (n < 0)
			continue;
		for (size_t i = 0; i < pfds.size(); ++i)
		{
			if (pfds[i].revents == 0 || _handlers.find(pfds[i].fd) == _handlers.end())
				continue;
			int fd = pfds[i].fd;
			Event ev = pfds[i].revents;
			if (ev & POLLERR)  //RST 受信（コレクション異常終了時）
			{
				RemoveHandler(_handlers[fd]);
				continue;
			}
			if (ev & POLLIN || ev & POLLHUP)  //POLLHUP: CGI終了の検知
				_handlers[fd]->ReadEvent(ev);
			if (ev & POLLOUT)
				_handlers[fd]->WriteEvent(ev);
		}
	}
}

void ServerLoop::RegisterHandler(AFdhandler *h)
{
	std::cout << "RegisterHandler fd = " << h->GetFd() << std::endl;
	_handlers[h->GetFd()] = h;
}

void ServerLoop ::RemoveHandler(AFdhandler *h)
{
	std::cout << "RemoveHandler fd = " << h->GetFd() << std::endl;
	_handlers.erase(h->GetFd());
	delete h;
}

void ServerLoop::ModifyHandler(AFdhandler *h, Event new_event, int new_fd)
{
	std::cout << "[ befor ] ModifyHandler fd = " << h->GetFd() << std::endl;
	_handlers.erase(h->GetFd());
	h->SetEvent(new_event);
	h->SetFd(new_fd);
	_handlers[h->GetFd()] = h;
	std::cout << "[ after ] ModifyHandler fd = " << h->GetFd() << std::endl;
}
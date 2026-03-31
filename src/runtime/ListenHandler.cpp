#include "ListenHandler.hpp"
#include "ConnectionHandler.hpp"
#include "ServerLoop.hpp"
#include "Utils.hpp"

ListenHandler::ListenHandler(int fd, ServerLoop &loop, std::map< std::string, ServerConfig > &confs) :
	AFdhandler(fd, POLLIN, loop), _confs(confs)
{
}

ListenHandler::~ListenHandler()
{
	safty_close(_fd);
	return;
}

void ListenHandler::ReadEvent(Event ev)
{
	(void)ev;
	while (1)
	{
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		int new_fd = accept(GetFd(), (struct sockaddr *)&addr, &addrlen);
		if (new_fd <= 0)
			return;	 //error
		int frags = fcntl(new_fd, F_GETFL, 0);
		fcntl(new_fd, F_SETFL, frags | O_NONBLOCK);
		fcntl(new_fd, F_SETFD, FD_CLOEXEC);
		struct in_addr a = addr.sin_addr;
		unsigned char *p = (unsigned char *)&a.s_addr;
		//
		std::ostringstream oss;
		oss << (int)p[0] << "."
			<< (int)p[1] << "."
			<< (int)p[2] << "."
			<< (int)p[3];
		//
		std::string client_ip = oss.str();
		ConnectionHandler *h = new ConnectionHandler(new_fd, _loop, _confs, client_ip);
		std::cout << "Connection OK" << std::endl;
		std::cout << "IP = " << oss.str() << std::endl;
		std::cout << "Port = " << ntohs(addr.sin_port) << std::endl;
		_loop.RegisterHandler(h);
		std::cout << "-------------------" << std::endl;
	}
}

void ListenHandler::WriteEvent(Event ev)
{
	(void)ev;
	return;
}
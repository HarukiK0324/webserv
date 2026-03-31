#ifndef LISTENHANDLER_HPP
#define LISTENHANDLER_HPP
#include <map>
#include <string>
#include <iostream>
#include <netdb.h>
#include <fcntl.h>
#include "AFdHandler.hpp"

struct ServerConfig;

//Listen Socket　ポートを開いて、接続要求（SYN）を待つソケット
class ListenHandler : public AFdhandler
{
private:
	std::map< std::string, ServerConfig > &_confs;

public:
	ListenHandler(int fd, ServerLoop &loop, std::map< std::string, ServerConfig > &confs);
	virtual ~ListenHandler();
	virtual void ReadEvent(Event ev);
	virtual void WriteEvent(Event ev);
};

#endif
#ifndef PORTMANAGER_HPP
#define PORTMANAGER_HPP
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "ServerConfig.hpp"
//Congig から必要な情報 port ip addr ip通信のver
// 必要な情報　port_num host_ip
class PortManager
{
private:
	//first->fd  second->(ServerName(Host header name), ServerConfig)
	std::map< int, std::map< std ::string, ServerConfig > > _PortfdAndConfs;

public:
	PortManager();
	PortManager(const std::map< ListenKey, std::vector< ServerConfig > > &ServerConfigs);
	~PortManager();
	std::map< int, std::map< std ::string, ServerConfig > > &getPortfdAndConfs();
	void MakePortfdAndConfs(int fd, std::vector< ServerConfig > const &sconfs);
};

#endif
// PortManager(const std::map<int, std::string> &SoketArg);
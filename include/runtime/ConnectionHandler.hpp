#ifndef CONNECTIONHANDLER_HPP
#define CONNECTIONHANDLER_HPP

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include "AFdHandler.hpp"
#include "HttpProtocol.hpp"

#define CON_TIME_LIMIT 10
class ServerLoop;
class CgiHandler;
class HttpProtocol;

// Connected Socket クライアント1人につき1つずつ生成される
// リクエストの読み取り、レスポンスの送信を担当
// try catchはここで行って不正なときはそれをSEND（）するようにする
class ConnectionHandler : public AFdhandler
{
	enum STATE
	{
		READING,
		WRITING,
		TIMEOUT,
	};

private:
	time_t _lastActive;
	time_t _limittime;

	std::string _client_ip;
	unsigned int _offset;
	STATE _state;
	HttpProtocol _httpProtocol;
	void SetState(STATE state)
	{
		_state = state;
	};
	void SetEventPollout()
	{
		_event = POLLOUT;
	};
	void UnSetEventPollout()
	{
		_event = POLLIN;
	};

public:
	ConnectionHandler(int fd, ServerLoop &loop, std::map< std::string, ServerConfig > &conf, const std::string &client_ip);
	virtual ~ConnectionHandler();
	const std::string &GetClientIp() const
	{
		return _client_ip;
	}
	virtual void ReadEvent(Event ev);
	virtual void WriteEvent(Event ev);
	virtual void CheckTimeout(const time_t &_time);
};

// KEEP_CONNECTION,
//void SetLastTime();
#endif
#include "ConnectionHandler.hpp"
#include "ServerLoop.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdio>
#include <mach/error.h>

int PtoCpipes[2];
int CtoPpipes[2];

ConnectionHandler ::ConnectionHandler(int fd, ServerLoop &loop, std::map< std::string, ServerConfig > &conf, const std::string &client_ip) : AFdhandler(fd, POLLIN, loop),
																																			 _client_ip(client_ip),
																																			 _offset(0),
																																			 _state(READING),
																																			 _httpProtocol(loop, conf, client_ip)

{
	std::cout << "-----------------------" << std::endl;
	std::cout << "new ConnectionHandler" << std::endl;
	std::cout << "-----------------------" << std::endl;
	_lastActive = (std::time(NULL));
	_limittime = CON_TIME_LIMIT;
}
ConnectionHandler ::~ConnectionHandler()
{
	safty_close(_fd);
	std::cout << "ConnectionHandler: Connection closed." << std::endl;
	std::cout << "-----------------------" << std::endl;
}

void ConnectionHandler ::CheckTimeout(const time_t &_time)
{
	if (_state == TIMEOUT)
		return;
	if (_time - _lastActive >= _limittime)	//リクエストREAD()の完了までの時間制限
	{
		SetState(TIMEOUT);
		SetEventPollout();
	}
}

void ConnectionHandler ::ReadEvent(Event ev)
{
	(void)ev;
	_httpProtocol._inbuf.clear();
	_httpProtocol._inbuf.resize(BUFLEN);
	ssize_t n = recv(_fd, _httpProtocol._inbuf.data(), BUFLEN, 0);
	std::cout << "recv:" << n << std::endl;
	if (n > 0)
	{
		_httpProtocol._inbuf.resize(n);
		std::string readstring = std::string(_httpProtocol._inbuf.begin(), _httpProtocol._inbuf.end());
		_httpProtocol.DoProcess();
		if (_httpProtocol.getState() != HttpProtocol::PARSING)
		{
			std::cout << "ConnectionHandler: Parse OK SetState Writing" << std::endl;
			SetEventPollout();
			SetState(WRITING);
			return;
		}
	}
	else if (n == 0)  //　相手側　正常終了ケース、writeのみ閉じたか、r＆w閉じたか）
	{
		std::cout << "ConnectionHandler: Peer closed connection." << std::endl;	 // No more data from peer. Close to avoid keeping a dead connection.
		_loop.RemoveHandler(this);
		return;
	}
	else if (n < 0)	 //　割り込み（シグナル）または、空なのにrecvしたとき
		return;
}

void ConnectionHandler ::WriteEvent(Event ev)
{
	(void)ev;
	switch (_state)
	{
		case WRITING:
		{
			_httpProtocol.DoProcess();
			if (_httpProtocol.getState() == HttpProtocol::END_PROCESS)
			{
				while (_offset < _httpProtocol._outbuf.size())
				{
					ssize_t n = send(_fd, _httpProtocol._outbuf.data() + _offset, _httpProtocol._outbuf.size() - _offset, MSG_NOSIGNAL);
					if (n < 0)
						return;
					_offset += n;
				}
				if (_offset == _httpProtocol._outbuf.size())
				{
					std::cout << "ConnectionHandler: Response sent, closing connection." << std::endl;
					_loop.RemoveHandler(this);
					return;
				}
			}
			break;
		}
		case TIMEOUT:
		{
			_httpProtocol.TimeoutProcess();
			_offset = 0;
			while (_offset < _httpProtocol._outbuf.size())
			{
				std::cerr << "Timeout WriteEvent Response: " << std::endl;
				ssize_t n = send(_fd, _httpProtocol._outbuf.data() + _offset, _httpProtocol._outbuf.size() - _offset, MSG_NOSIGNAL);
				if (n < 0)
					return;
				_offset += n;
			}
			if (_offset == _httpProtocol._outbuf.size())
			{
				std::cout << "ConnectionHandler: Timeout response sent, closing connection." << std::endl;
				_loop.RemoveHandler(this);
				return;
			}
			break;
		}
		default:
			break;
	}
}
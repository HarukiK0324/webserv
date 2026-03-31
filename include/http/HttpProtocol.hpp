#ifndef HTTPPROTOCOL_HPP
#define HTTPPROTOCOL_HPP

#include "HttpParser.hpp"
#include "CgiHandler.hpp"
#include "Method.hpp"
#include "GetMethod.hpp"
#include "PostMethod.hpp"
#include "DeleteMethod.hpp"
#include <iostream>

#define CGI_COUNT_LIMIT 5
#define SESSION_TIMEOUT_SECONDS 3600
class ServerLoop;
class CgiHandler;

class HttpProtocol
{
public:
	enum State
	{
		PARSING,
		METHOD_PROCESSING,
		BUILDING_RESPONSE,
		END_PROCESS,
		WAIT_CGI,
	};

private:
	ServerLoop &_loop;
	std::map< std::string, ServerConfig > &_config;
	std::string _client_ip;
	State _state;
	HttpParser _httpParser;
	HttpResponse _httpResponse;
	HttpRequest _httpRequest;
	CgiHandler *_cgiHandler;
	int _cgi_count;
	std::vector< char > _inbuf;
	std::vector< char > _outbuf;
	//
	void SetState(State state);
	State getState() const;
	void StaticMethod(LocationConfig &lconfig);
	void SetErrorPage(int status_code);

	friend class ConnectionHandler;	 //_inbuf _outbufへのアクセスのため
public:
	HttpProtocol(ServerLoop &loop, std::map< std::string, ServerConfig > &config, const std::string &client_ip);
	~HttpProtocol();
	void DoProcess();
	void TimeoutProcess();
};

std::ostream &operator<<(std::ostream &os, const HttpProtocol::State &state);

#endif

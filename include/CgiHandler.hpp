#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <wait.h>
#include "FdHandler.hpp"
#include "CgiParser.hpp"
#include "LocationConfig.hpp"

struct ServerConfig;
class ServerLoop;
class HttpRequest;

class CgiHandler : public FdHandler
{
private:
	int _pipe_to_child[2];	  // 親 → 子 (stdin)
	int _pipe_from_child[2];  // 子 → 親 (stdout)
	LocationConfig _lconf;

	bool _end;
	bool _permission_denied;
	pid_t childlen;
	std::vector< char > _inbuf;
	time_t _lastActive;
	time_t _limittime;

	std::vector< char > _body;
	std::map< std::string, std::string > _envs;
	std::string _file_path;

	void killProcess();
	void ChildProcess();
	void ParentProcess();

public:
	CgiParser _parser;
	CgiHandler(const LocationConfig &lconf, ServerLoop &loop);
	~CgiHandler();
	void MakeEnvp(const HttpRequest &request, const std::string &client_ip, const std::string &port);
	virtual void ReadEvent(Event ev);
	virtual void WriteEvent(Event ev);
	void PrintEnvp();
	bool IsCgiEnd() const
	{
		return _end;
	};	//time only
	bool IsTimedOut(const time_t &now) const
	{
		return (now - _lastActive >= _limittime);
	};	// CGI 専用タイムアウト判定
};

#endif

//ConnectionHandlerでthisを作成して、子プロセスで実行、_reusltで結果を送信する timeoutの管理もできている
//http parser 終了後　Locationconfig決定する.
// targetURIをもとにして、実行するファイルパスの決定　configのrootとの結合をして実行する

// 	meta - variable - name = "AUTH_TYPE" | "CONTENT_LENGTH" |
// 	"CONTENT_TYPE" | "GATEWAY_INTERFACE" |
// 	"PATH_INFO" | "PATH_TRANSLATED" |
// 	"QUERY_STRING" | "REMOTE_ADDR" |
// 	"REMOTE_HOST" | "REMOTE_IDENT" |
// 	"REMOTE_USER" | "REQUEST_METHOD" |
// 	"SCRIPT_NAME" | "SERVER_NAME" |
// 	"SERVER_PORT" | "SERVER_PROTOCOL" |
// 	"SERVER_SOFTWARE" | scheme | protocol - var - name | extension - var - name
//	protocol - var - name = (protocol | scheme) "_" var - name
//	scheme = alpha * (alpha | digit | "+" | "-" | ".")
//	var - name = token
//	extension-var-name = token 																																												  extension -

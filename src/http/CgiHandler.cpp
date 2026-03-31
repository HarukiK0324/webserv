#include "CgiHandler.hpp"
#include "LocationConfig.hpp"
#include "Utils.hpp"
#include "HttpRequest.hpp"
#include "ServerLoop.hpp"

#define CGI_TIME_LIMIT 10

namespace
{
bool is_cgi_script(const std::string &path, const LocationConfig &lconf)
{
	if (path.size() >= lconf.cgi_type.size() &&
		path.compare(path.size() - lconf.cgi_type.size(), lconf.cgi_type.size(), lconf.cgi_type) == 0)
		return true;
	return false;
}
}  // namespace

CgiHandler::CgiHandler(const LocationConfig &lconf, ServerLoop &loop) :
	AFdhandler(-1, POLLOUT, loop), _lconf(lconf), _end(false), _permission_denied(false), childlen(-1), _parser()
{
	if (pipe(_pipe_to_child) < 0)
		throw std::runtime_error("pipe to child");
	if (pipe(_pipe_from_child) < 0)
		throw std::runtime_error("pipe from child");
	_fd = _pipe_to_child[WRITE_FD];
	fcntl(_pipe_to_child[0], F_SETFL, O_NONBLOCK);
	fcntl(_pipe_to_child[1], F_SETFL, O_NONBLOCK);
	fcntl(_pipe_from_child[0], F_SETFL, O_NONBLOCK);
	fcntl(_pipe_from_child[1], F_SETFL, O_NONBLOCK);
	_limittime = CGI_TIME_LIMIT;
	_lastActive = std::time(NULL);
	_loop.RegisterHandler(this);
}

CgiHandler::~CgiHandler()
{
	killProcess();
	waitpid(childlen, NULL, WNOHANG);
	safty_close(_pipe_to_child[READ_FD]);
	safty_close(_pipe_to_child[WRITE_FD]);
	safty_close(_pipe_from_child[READ_FD]);
	safty_close(_pipe_from_child[WRITE_FD]);
}
void CgiHandler ::ReadEvent(Event ev)
{
	(void)ev;
	if (_permission_denied)
	{
		std::cerr << "Permission denied for CGI execution: " << _lconf.CgiExecuter << " or " << _file_path << std::endl;
		_parser.setStatusCode(CgiParser::FORBIDDEN);
		_end = true;
		SetEvent(0);
		return;
	}
	_inbuf.clear();
	_inbuf.resize(BUFLEN);
	ssize_t n = read(_fd, _inbuf.data(), BUFLEN);  // change  CRLF to LF
	if (n < 0)
		return;
	else if (n > 0)
		_parser.ReadResponse(_inbuf.data(), n);
	else if (n == 0)  //EOF POLLHUP
		_parser.ReadResponse(NULL, 0);
	if (_parser.getState() == CgiParser::DONE || _parser.getState() == CgiParser::ERROR)
	{
		_end = true;
		SetEvent(0);
		return;
	}
}

void CgiHandler ::WriteEvent(Event ev)
{
	//std::cerr << "CgiHandler: WriteEvent called with event " << ev << std::endl;
	if (ev & POLLOUT)  // 子プロセスへの標準入力書き込み
	{
		if (access(_lconf.CgiExecuter.c_str(), X_OK) == -1 || access(_file_path.c_str(), R_OK) == -1)
			_permission_denied = true;
		if ((childlen = fork()) < 0)
			throw std::runtime_error("fork");
		if (childlen == 0)
			ChildProcess();
		else
			ParentProcess();
	}
}
void CgiHandler ::ChildProcess()
{
	dup2(_pipe_to_child[READ_FD], STDIN_FILENO);
	dup2(_pipe_from_child[WRITE_FD], STDOUT_FILENO);
	safty_close(_pipe_to_child[WRITE_FD]);
	safty_close(_pipe_to_child[READ_FD]);
	safty_close(_pipe_from_child[WRITE_FD]);
	safty_close(_pipe_from_child[READ_FD]);
	if (_permission_denied)
		std::exit(1);
	char *argv[] = {(char *)_lconf.CgiExecuter.c_str(), (char *)_file_path.c_str(), NULL};
	std::vector< char * > _envp;
	size_t pos = _file_path.rfind('/');
	std::string cgi_dir;
	if (pos != std::string::npos)
		cgi_dir = _file_path.substr(0, pos);
	else
		std::exit(1);
	if (chdir(cgi_dir.c_str()) != 0)
		std::exit(1);
	for (std::map< std::string, std::string >::iterator it = _envs.begin(); it != _envs.end(); ++it)
	{
		std::string env = it->first + "=" + it->second;
		_envp.push_back(strdup(env.c_str()));
	}
	_envp.push_back(NULL);
	execve(_lconf.CgiExecuter.c_str(), argv, _envp.data());
}
void CgiHandler ::ParentProcess()
{
	_lastActive = std::time(NULL);
	if (!_permission_denied && _envs["REQUEST_METHOD"] == "POST" && _envs["CONTENT_LENGTH"] != "")
		write(_pipe_to_child[WRITE_FD], _body.data(), _body.size());
	safty_close(_pipe_to_child[READ_FD]);
	safty_close(_pipe_from_child[WRITE_FD]);
	safty_close(_pipe_to_child[WRITE_FD]);
	_loop.ModifyHandler(this, POLLIN, _pipe_from_child[READ_FD]);
}

void CgiHandler ::MakeEnvp(const HttpRequest &request, const std::string &client_ip, const ServerConfig &config)
{
	std ::string body = request.getBody();
	//定数
	_envs["AUTH_TYPE"] = "";
	_envs["GATEWAY_INTERFACE"] = "CGI/1.1";
	_envs["SERVER_SOFTWARE"] = "webserv/1.0";
	_envs["SERVER_PORT"] = to_string(config.port);
	std::cerr << "SERVER_NAME:" << request.getHeader("host") << std::endl;
	std::string host_header = request.getHeader("host");
	if (host_header.empty() && !config.server_name.empty())
		host_header = config.server_name[0];
	else if (host_header.empty() && config.server_name.empty())
		host_header = config.listen_host == "*" ? "localhost" : config.listen_host;
	else
		host_header = host_header.substr(0, host_header.find(':'));
	_envs["SERVER_NAME"] = host_header;	 // どうするか？
	//startlineから
	_envs["REQUEST_METHOD"] = request.getMethod();
	_envs["SERVER_PROTOCOL"] = request.getProtocol();
	std::string query_string;
	decodePercentEncoded(request.getQueryString(), query_string);
	_envs["QUERY_STRING"] = query_string;
	//REMOTE系
	_envs["REMOTE_ADDR"] = client_ip;  //hostnumber   = ipv4-address | ipv6-address
	_envs["REMOTE_HOST"] = client_ip;
	_envs["REMOTE_IDENT"] = "";	 //からで大丈夫
	_envs["REMOTE_USER"] = "";	 //未対応 認証系の情報　
	//headerから、読み取り
	if (!body.empty() || request.getHeader("content-length") != "" || request.getHeader("transfer-encoding") != "")
	{
		std::stringstream ss;
		ss << body.size();
		_envs["CONTENT_LENGTH"] = ss.str();
	}
	if (request.getHeader("content-type") != "")
		_envs["CONTENT_TYPE"] = request.getHeader("content-type");
	else
		_envs["CONTENT_TYPE"] = "";
	if (request.getHeader("cookie") != "")
		_envs["HTTP_COOKIE"] = request.getHeader("cookie");
	std::string targetURI = request.getPath();	//クエリとパスを分けて、
	if (request.getQueryString() != "")
	{
		size_t pos = targetURI.find(request.getQueryString());
		if (pos != std::string::npos)
			targetURI = targetURI.substr(0, pos - 1);
	}
	std::string script_name;
	std::string path_info;
	for (size_t pos = 1; pos <= targetURI.size(); ++pos)
	{
		if (pos == targetURI.size() || targetURI[pos] == '/')
		{
			std::string candidate = targetURI.substr(0, pos);
			if (is_cgi_script(candidate, _lconf))
			{
				script_name = candidate;
				path_info = targetURI.substr(pos);
				break;
			}
		}
	}
	_envs["PATH_INFO"] = path_info;
	_envs["SCRIPT_NAME"] = script_name;
	_envs["PATH_TRANSLATED"] = _lconf.root + path_info;
	_file_path = _lconf.root + script_name;	 //script_name
	if (body.size() > 0)
	{
		_body.reserve(body.size());
		for (size_t i = 0; i < body.size(); i++)
		{
			_body.push_back(body[i]);
		}
	}
}

void CgiHandler ::killProcess()
{
	if (childlen > 0)
		kill(childlen, SIGKILL);
}

void CgiHandler ::PrintEnvp()
{
	for (std::map< std::string, std::string >::const_iterator it = _envs.begin(); it != _envs.end(); ++it)
	{
		std::cout << "_envs[" << it->first << "] = " << it->second << std::endl;
	}
}

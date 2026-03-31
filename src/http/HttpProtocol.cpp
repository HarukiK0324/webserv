#include "HttpProtocol.hpp"
#include "ServerLoop.hpp"
#include "Utils.hpp"

namespace
{

void SetServerConfig(const HttpRequest &request, std::map< std::string, ServerConfig > &config, ServerConfig &default_serverconf)
{
	default_serverconf = config.begin()->second;  //default server
	for (std::map< std::string, ServerConfig >::iterator it = config.begin(); it != config.end(); ++it)
	{
		if (it->first == request.getHeader("host"))
			default_serverconf = (it->second);
	}
}

void SetLocationConfig(const HttpRequest &request, std::map< std::string, ServerConfig > &config, LocationConfig &default_location)
{
	ServerConfig target_server;
	SetServerConfig(request, config, target_server);
	std::string path = request.getPath();
	for (std::map< std::string, LocationConfig >::const_iterator it = target_server.locations.begin();
		it != target_server.locations.end();
		++it)
	{
		if (!it->first.empty() && (path == it->first || (it->first != "/" && path.find(it->first) == 0)))
		{
			default_location = it->second;
			return;
		}
	}
	default_location = target_server.locations[""];
}
bool IsCgi(const HttpRequest &request, LocationConfig &location)
{
	if (location.cgi_type.empty() || location.CgiExecuter.empty())
		return false;
	std::string targetURI = request.getPath();
	if (request.getQueryString() != "")
	{
		size_t pos = targetURI.find(request.getQueryString());
		if (pos != std::string::npos)
			targetURI = targetURI.substr(0, pos - 1);
	}
	for (size_t pos = 1; pos <= targetURI.size(); ++pos)
	{
		if (pos == targetURI.size() || targetURI[pos] == '/')
		{
			std::string candidate = targetURI.substr(0, pos);
			if (candidate.size() >= location.cgi_type.size() &&
				candidate.compare(candidate.size() - location.cgi_type.size(), location.cgi_type.size(), location.cgi_type) == 0)
				return true;
		}
	}
	return false;
}

bool checkAllowMethod(const std::string &method, const LocationConfig &location)
{
	for (std::vector< std::string >::const_iterator it = location.allow_methods.begin(); it != location.allow_methods.end(); ++it)
	{
		if (*it == method)
			return true;
	}
	return false;
}
}  // namespace

HttpProtocol::HttpProtocol(ServerLoop &loop, std::map< std::string, ServerConfig > &config, const std::string &client_ip) : _loop(loop),
																															_config(config),
																															_client_ip(client_ip),
																															_state(PARSING),
																															_httpParser(config.begin()->second),
																															_httpResponse(),
																															_httpRequest(),
																															_cgiHandler(NULL),
																															_cgi_count(0)
{
}

HttpProtocol ::~HttpProtocol()
{
	if (_cgiHandler)
	{
		_loop.RemoveHandler(_cgiHandler);
	}
}

void HttpProtocol::TimeoutProcess()	 // リクエストのREAD()が完了するまでの時間の制限
{
	_outbuf.clear();
	_httpResponse = HttpResponse();
	_httpResponse.setStatusCode(408);
	_httpResponse.setErrorBody("");
	_httpResponse.setHeader("Connection", "close");
	std::string response = _httpResponse.toString();
	_outbuf.insert(_outbuf.end(), response.begin(), response.end());
	SetState(END_PROCESS);
}

void HttpProtocol::SetState(State state)
{
	std::cout << "State changed: " << _state << " -> " << state << std::endl;
	_state = state;
}

HttpProtocol::State HttpProtocol::getState() const
{
	return _state;
}

void HttpProtocol::DoProcess()
{
	switch (getState())
	{
		case HttpProtocol::PARSING:
		{
			_httpParser.sendData(_inbuf.data(), _inbuf.size());
			if (_httpParser.getState() == ERROR)
			{
				_httpResponse = _httpParser.getResponse(_config.begin()->second);
				SetState(BUILDING_RESPONSE);
			}
			else if (_httpParser.getState() == DONE)
			{
				_httpRequest = _httpParser.getRequest();
				std::string session_id = _httpRequest.getCookie("session_id");
				bool is_existing = !session_id.empty() && _loop.hasSession(session_id);
				bool session_valid = is_existing && _loop.getSessions(session_id).getExpiresAt() >= std::time(NULL);
				if (!session_valid)
				{
					if (is_existing)
					{
						std::cout << "Session expired: " << session_id << std::endl;
						_loop.removeSession(session_id);
					}
					session_id = Session::getNewSessionId();
					for (int retry = 0; _loop.hasSession(session_id) && retry < 100; ++retry)
						session_id = Session::getNewSessionId();
					if (_loop.hasSession(session_id))
					{
						_httpResponse.setStatusCode(503);
						SetState(BUILDING_RESPONSE);
						break;
					}
					_loop.setSessions(session_id, Session(session_id, std::time(NULL) + SESSION_TIMEOUT_SECONDS, std::map< std::string, std::string >()));
					_httpResponse.setHeader("Set-Cookie", "session_id=" + session_id + "; Max-Age=" + to_string(SESSION_TIMEOUT_SECONDS) + "; Path=/; HttpOnly; SameSite=Lax");
					_httpRequest.setSessionId(session_id);
				}
				else
				{
					std::cout << "Session valid: " << session_id << std::endl;
					_loop.getSessions(session_id).setExpiresAt(std::time(NULL) + SESSION_TIMEOUT_SECONDS);
					_httpRequest.setSessionId(session_id);
				}
				SetState(METHOD_PROCESSING);
			}
			break;
		}
		case HttpProtocol::METHOD_PROCESSING:
		{
			// location特定とPATHの検出
			const std::string &method = _httpRequest.getMethod();
			if (!(method == "GET" || method == "POST" || method == "DELETE"))
			{
				_httpResponse.setStatusCode(501);
				SetState(BUILDING_RESPONSE);
				break;
			}
			LocationConfig lconfig;
			std::cout << "-----------------------" << std::endl;
			std::cout << "RequestPath: " << _httpRequest.getPath() << std::endl;
			std::cout << "-----------------------" << std::endl;
			SetLocationConfig(_httpRequest, _config, lconfig);
			if (IsCgi(_httpRequest, lconfig))  //CGI
			{
				if (CGI_COUNT_LIMIT < _cgi_count++)
				{
					std::cerr << "CGI execution count limit exceeded" << std::endl;
					_httpResponse.setStatusCode(500);
					SetState(BUILDING_RESPONSE);
					break;
				}
				std::string str;
				if (!decodePercentEncoded(_httpRequest.getQueryString(), str))
				{
					_httpResponse.setStatusCode(400);
					SetState(BUILDING_RESPONSE);
					break;
				}
				ServerConfig sconf;
				SetServerConfig(_httpRequest, _config, sconf);
				_cgiHandler = new CgiHandler(lconfig, _loop);
				_cgiHandler->MakeEnvp(_httpRequest, _client_ip, sconf);
				SetState(WAIT_CGI);
			}
			else  //Static Method
			{
				StaticMethod(lconfig);
				SetState(BUILDING_RESPONSE);
			}
			break;
		}
		case HttpProtocol::BUILDING_RESPONSE:
		{
			int status_code = _httpResponse.getStatusCode();
			if (status_code >= 400)
				SetErrorPage(status_code);
			std::string response = _httpResponse.toString();
			_outbuf.insert(_outbuf.end(), response.begin(), response.end());
			SetState(END_PROCESS);
			break;
		}
		case HttpProtocol::END_PROCESS:
			break;
		case HttpProtocol::WAIT_CGI:  // CgiParser編集必要あり
		{
			if (_cgiHandler->IsCgiEnd())
			{
				if (_cgiHandler->_parser.getResuponseType() == CgiParser::LOCAL_REDIR_RESPONSE)
				{
					//CGIローカルリダイレクト　パスだけ変えて再度命令実行
					_httpRequest.setPath(_cgiHandler->_parser.getHeaders().find("Location")->second);
					std::cout << "Local Redirect to: " << _httpRequest.getPath() << std::endl;
					std::cout << _cgiHandler->_parser.getHeaders().find("Location")->second << std::endl;
					SetState(METHOD_PROCESSING);
				}
				else
				{
					//CGI正常終了
					_httpResponse.setStatusCode(_cgiHandler->_parser.getStatusCode());
					const std::vector< char > &body = _cgiHandler->_parser.getBody();
					_httpResponse.setBody(std::string(body.begin(), body.end()));
					for (std::map< std::string, std::string >::const_iterator it = _cgiHandler->_parser.getHeaders().begin();
						it != _cgiHandler->_parser.getHeaders().end();
						++it)
					{
						_httpResponse.setHeader(it->first, it->second);
					}
					SetState(BUILDING_RESPONSE);
				}
				_loop.RemoveHandler(_cgiHandler);
				_cgiHandler = NULL;
			}
			else if (_cgiHandler->IsTimedOut(std::time(NULL)))
			{
				_httpResponse.setStatusCode(504);
				SetState(BUILDING_RESPONSE);
				_loop.RemoveHandler(_cgiHandler);
				_cgiHandler = NULL;
			}
			break;	//waiting
		}
		default:
			return;
	}
}

void HttpProtocol::StaticMethod(LocationConfig &lconfig)
{
	if (lconfig.redirect && _httpRequest.getPath().find(lconfig.path) == 0)
	{
		std::cout << "Redirecting to: " << lconfig.redirect_target << std::endl;
		_httpResponse.setStatusCode(lconfig.redirect_code);
		_httpResponse.setHeader("Location", lconfig.redirect_target);
		return;
	}
	if (_httpRequest.getMethod() == "GET" && checkAllowMethod("GET", lconfig))
	{
		GetMethod method;
		method.execute(_httpRequest, _httpResponse, lconfig);
	}
	else if (_httpRequest.getMethod() == "POST" && checkAllowMethod("POST", lconfig))
	{
		PostMethod method;
		method.execute(_httpRequest, _httpResponse, lconfig);
	}
	else if (_httpRequest.getMethod() == "DELETE" && checkAllowMethod("DELETE", lconfig))
	{
		DeleteMethod method;
		method.execute(_httpRequest, _httpResponse, lconfig);
	}
	else  //method not allowed
	{
		std::string method;
		for (std::vector< std::string >::iterator it = lconfig.allow_methods.begin(); it != lconfig.allow_methods.end(); ++it)
			method += *it + ", ";
		_httpResponse.setHeader("Allow", method.substr(0, method.size() - 2));
		_httpResponse.setStatusCode(405);
	}
}
//CONFIGにエラーページが設定されている場合は、エラーページの内容をレスポンスにセットする
//それ以外は任意のHTML送信

void HttpProtocol::SetErrorPage(int StatusCode)
{
	ServerConfig s = _config.begin()->second;
	std::cerr << "Setting error page for status code: " << StatusCode << std::endl;
	if (_config.find(_httpRequest.getHeader("host")) != _config.end())
		s = _config[_httpRequest.getHeader("host")];
	if ((s.error_page.find(StatusCode) != s.error_page.end() && s.locations.find(s.error_page[StatusCode]) != s.locations.end()))
	{
		std::cerr << "Setting custom error page for status code: " << StatusCode << std::endl;
		LocationConfig &lconf = s.locations[s.error_page[StatusCode]];
		std::string full_path = lconf.root + lconf.path;
		std::ifstream file(full_path.c_str());
		if (file)
		{
			std::ostringstream ss;
			ss << file.rdbuf();
			_httpResponse.setBody(ss.str());
			_httpResponse.setHeader("Content-Type", MimeTypes::getType(full_path));
			file.close();
			return;
		}
	}
	// setStatusCode()を呼んで_statusMessageをリセット（HttpParserで上書きされているため）
	_httpResponse.setStatusCode(StatusCode);
	_httpResponse.setErrorBody("");
	return;
}
std::ostream &operator<<(std::ostream &os, const HttpProtocol::State &state)
{
	switch (state)
	{
		case HttpProtocol::PARSING:
			os << "PARSING";
			break;
		case HttpProtocol::METHOD_PROCESSING:
			os << "METHOD_PROCESSING";
			break;
		case HttpProtocol::BUILDING_RESPONSE:
			os << "BUILDING_RESPONSE";
			break;
		case HttpProtocol::END_PROCESS:
			os << "END_PROCESS";
			break;
		case HttpProtocol::WAIT_CGI:
			os << "WAIT_CGI";
			break;
		default:
			os << "UNKNOWN_STATE";
			break;
	}
	return os;
}
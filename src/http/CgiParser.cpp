#include "CgiParser.hpp"
#include <cstdlib>
// Location = local - Location | client - Location
// client - Location = "Location:" fragment - URI NL
// local - Location = "Location:" local - pathquery NL

//　local系のときの path
// local - pathquery = abs - path["?" query - string]
// abs - path = "/" path - segments　絶対パス　関数作成時に重要な文法
// path - segments = segment * ("/" segment)
// segment = *pchar
// pchar = unreserved | escaped | extra extra = ":" | "@" | "&" | "=" | "+" | "$" | "," 予約語以外何でもOK

// client系のときの　URI
// fragment - URI = absoluteURI["#" fragment]
// absoluteURI = scheme ":" hier - part [ "?" query - string ]
// scheme = alpha *( alpha | digit | "+" | "-" | "." )
// hier - part = "//" authority [ path - absolute ] | path - absolute

namespace
{

bool IsHttpOrHttpsAbsoluteURI(const std::string &uri)
{
	if (uri.compare(0, 7, "http://") != 0 && uri.compare(0, 8, "https://") != 0)
		return false;
	return uri.size() > uri.find("://") + 3;
}

bool IsAbsPath(const std::string &path)
{
	if (path.empty() || path[0] != '/')
		return false;
	return true;
}
size_t end_of_headers(const std::vector< char > &buffer, size_t pos)
{
	while (pos < buffer.size())
	{
		if (pos + 3 < buffer.size() &&
			buffer[pos] == '\r' && buffer[pos + 1] == '\n' &&
			buffer[pos + 2] == '\r' && buffer[pos + 3] == '\n')
		{
			return pos + 3;
		}
		if (pos + 1 < buffer.size() &&
			buffer[pos] == '\n' && buffer[pos + 1] == '\n')
		{
			return pos + 1;
		}
		++pos;
	}
	return 0;
}
}  // namespace

CgiParser::CgiParser() : _state(HEADER_FIELDS), _responseType(UNKNOWN_RESPONSE), _statusCode(OK), _offset(0), _responseBuffer(), _body()
{
}

CgiParser::~CgiParser()
{
}

void CgiParser::ReadResponse(char *buf, size_t len)
{
	if (len == 0 && buf == NULL)  // EOF
	{
		switch (_state)
		{
			case HEADER_FIELDS:	 //NG
				setState(ERROR);
				setStatusCode(INTERNAL_SERVER_ERROR);
				break;
			case BODY:	//OK
				setState(DONE);
				break;
			default:
				break;
		}
		return;
	}
	switch (_state)
	{
		case HEADER_FIELDS:
		{
			_responseBuffer.reserve(_responseBuffer.size() + len);
			_responseBuffer.insert(_responseBuffer.end(), buf, buf + len);
			size_t pos = end_of_headers(_responseBuffer, _offset);
			if (pos != 0)
			{
				_offset = pos + 1;
				setState(BODY);
				_header_line = std::string(_responseBuffer.begin(), _responseBuffer.begin() + _offset);	 //改行２つ含む
				_body.insert(_body.end(), _responseBuffer.begin() + _offset, _responseBuffer.end());
				// std::cerr << "-----Header line------\n"
				// 		  << _header_line << std::endl
				// 		  << "---------------------" << std::endl;
				this->parser();
			}
			else
				_offset = _responseBuffer.size();
			return;
		}
		case BODY:
		{
			_body.reserve(_body.size() + len);
			_body.insert(_body.end(), buf, buf + len);
			return;
		}
		default:
			break;
	}
}
// headerの末尾の探索必要

void CgiParser::parser()
{
	try
	{
		// std::cerr << "-----start parser-----" << std::endl;
		if (_header_line.empty())
			throw std::runtime_error("CGI response missing required headers");
		std::string key, value;
		ParseHeaderKV(GetHeaderLine(), key, value);
		if (key == "Content-Type")
		{
			setResponseType(DOCUMENT_RESPONSE);
			setHeader(key, value);
			ReadHeaders();
			if (_headers.find("Location") != _headers.end())
				throw std::runtime_error("CGI response has both Content-Type and Location headers");
			if (_headers.find("Status") != _headers.end())
				setStatusCode(static_cast< StatusCode >(std::atoi(_headers["Status"].c_str())));
		}
		else if (key == "Location")
		{
			if (IsAbsPath(value))
				setResponseType(LOCAL_REDIR_RESPONSE);
			else if (IsHttpOrHttpsAbsoluteURI(value))
				setResponseType(CLIENT_REDIR_RESPONSE);
			else
				throw std::runtime_error("CGI Location hea== _header_line.size()der has invalid URI: " + value);
			setHeader(key, value);
			switch (_responseType)
			{
				case LOCAL_REDIR_RESPONSE:
					if (!(_header_line == (std::string(CRLF)) || _header_line == (std::string(LF))) && _responseBuffer.size() == _offset)
						throw std::runtime_error("CGI LOCAL REDIR response contains body data");
					break;
				case CLIENT_REDIR_RESPONSE:
					handleClientRedirect();
					break;
				default:
					break;
			}
		}
		else
		{
			setResponseType(UNKNOWN_RESPONSE);
			setState(ERROR);
			setStatusCode(INTERNAL_SERVER_ERROR);
		}
		//cgi のStatusヘッダは削除 httpresponseでは不要
		if (_headers.find("Status") != _headers.end())
			_headers.erase("Status");
		//std::cerr << "------end parser------" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "CgiParser Error: " << e.what() << std::endl;
		setResponseType(UNKNOWN_RESPONSE);
		setState(ERROR);
		setStatusCode(INTERNAL_SERVER_ERROR);
	}
}
std::string CgiParser::GetHeaderLine()
{
	std::string result;
	size_t line_end = _header_line.find(CRLF);
	size_t eol_len = 2;
	if (line_end == std::string::npos)
	{
		line_end = _header_line.find(LF);
		eol_len = 1;
	}
	if (line_end == std::string::npos)
		throw std::runtime_error("Malformed CGI header line: no line ending found");
	result = _header_line.substr(0, line_end);
	_header_line.erase(0, line_end + eol_len);
	// std::cerr << "GetHeaderLine: " << result << std::endl;
	return result;
}
void CgiParser::ParseHeaderKV(const std::string &line, std::string &key, std::string &value)
{
	if (line.empty())  //last line
	{
		// std::cerr << "ParseHeaderKV: Empty line, end of headers" << std::endl;
		key = "";
		value = "";
		return;
	}
	size_t colon_pos = line.find(COLON);
	if (colon_pos == std::string::npos)
		throw std::runtime_error("Malformed CGI header line: no colon found");
	key = line.substr(0, colon_pos);
	value = line.substr(colon_pos + 1);
	//trim spaces
	key.erase(0, key.find_first_not_of(SP));
	key.erase(key.find_last_not_of(SP) + 1);
	value.erase(0, value.find_first_not_of(SP));
	value.erase(value.find_last_not_of(SP) + 1);
	//
	if (key.empty() || value.empty())
		throw std::runtime_error("Malformed CGI header line: empty key or value");
	return;
}

void CgiParser::handleClientRedirect()
{
	// std::cerr << "CgiParser::handleClientRedirect: _headline: " << _header_line << std::endl;
	setStatusCode(FOUND);
	if (_header_line == (std::string(CRLF)) || _header_line == (std::string(LF)))  //Locationヘッダのみで、他のヘッダやボディがない場合
	{
		setResponseType(CLIENT_REDIR_RESPONSE);
		return;
	}
	std::string key, value;
	ParseHeaderKV(GetHeaderLine(), key, value);
	if (key == "Status")
	{
		int code = std::atoi(value.c_str());
		if (code != FOUND)
			throw std::runtime_error("CGI CLIENT REDIR response has invalid Status code: " + value);
		setHeader(key, value);
		std::string next_key, next_value;
		ParseHeaderKV(GetHeaderLine(), next_key, next_value);
		if (next_key == "Content-Type")
		{
			setHeader(next_key, next_value);
			ReadHeaders();
			setResponseType(CLIENT_REDIR_DOC_RESPONSE);
			return;
		}
		else
			throw std::runtime_error("CGI CLIENT REDIRDOC response missing Content-Type header");
	}
	else  //CLIENT_REDIR_RESPONSE  extension field
	{
		setResponseType(CLIENT_REDIR_RESPONSE);
		setHeader(key, value);
		ReadHeaders();
		if (_headers.find("Status") != _headers.end())
			throw std::runtime_error("CGI CLIENT REDIR response has unexpected Status header");
		if (_headers.find("Content-Type") != _headers.end())
			throw std::runtime_error("CGI CLIENT REDIR response has unexpected Content-Type header");
		return;
	}
}
// headerの末尾の探索必要
void CgiParser::ReadHeaders()
{
	std::string line;
	while (!_header_line.empty())
	{
		line = GetHeaderLine();
		if (line.empty())  //end of headers
			break;
		std::string key, value;
		ParseHeaderKV(line, key, value);  //keyとvalueは一時的にlineに格納
		setHeader(key, value);
	}
}

void CgiParser::setHeader(const std::string &key, const std::string &value)
{
	if (key == " Content-Type" || key == "Location" || key == "Status")
	{
		if (_headers.find(key) != _headers.end())
			throw std::runtime_error("Duplicate header field in CGI response: " + key);
	}
	_headers[key] = value;
}

void CgiParser::printResult()
{
	std::cout << "--------CgiParser Result--------" << std::endl;
	std::cout << "State: " << _state << std::endl;
	std::cout << "Response Type: " << _responseType << std::endl;
	std::cout << "Status Code: " << _statusCode << std::endl;
	std::cout << "<Headers>" << std::endl;
	for (std::map< std::string, std::string >::iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "<Body Size> " << _responseBuffer.size() - _offset << " bytes" << std::endl;
	std::cout << "--------------------------------" << std::endl;
}

std::ostream &operator<<(std::ostream &os, const CgiParser::State &state)
{
	switch (state)
	{
		case CgiParser::HEADER_FIELDS:
			os << "HEADER_FIELDS";
			break;
		case CgiParser::BODY:
			os << "BODY";
			break;
		case CgiParser::DONE:
			os << "DONE";
			break;
		case CgiParser::ERROR:
			os << "ERROR";
			break;
		default:
			os << "UNKNOWN_STATE";
			break;
	}
	return os;
}

std::ostream &operator<<(std::ostream &os, const CgiParser::StatusCode &statusCode)
{
	switch (statusCode)
	{
		case CgiParser::OK:
			os << "200 OK";
			break;
		case CgiParser::FOUND:
			os << "302 FOUND";
			break;
		case CgiParser::BAD_REQUEST:
			os << "400 BAD_REQUEST";
			break;
		case CgiParser::INTERNAL_SERVER_ERROR:
			os << "500 INTERNAL_SERVER_ERROR";
			break;
		case CgiParser::NOT_IMPLEMENTED:
			os << "501 NOT_IMPLEMENTED";
			break;
		case CgiParser::BAD_GATEWAY:
			os << "502 BAD_GATEWAY";
			break;
		case CgiParser::SERVICE_UNAVAILABLE:
			os << "503 SERVICE_UNAVAILABLE";
			break;
		default:
			os << "UNKNOWN_STATUS_CODE";
			break;
	}
	return os;
}

std::ostream &operator<<(std::ostream &os, const CgiParser::ResponseType &responsetype)
{
	switch (responsetype)
	{
		case CgiParser::DOCUMENT_RESPONSE:
			os << "DOCUMENT_RESPONSE";
			break;
		case CgiParser::LOCAL_REDIR_RESPONSE:
			os << "LOCAL_REDIR_RESPONSE";
			break;
		case CgiParser::CLIENT_REDIR_RESPONSE:
			os << "CLIENT_REDIR_RESPONSE";
			break;
		case CgiParser::CLIENT_REDIR_DOC_RESPONSE:
			os << "CLIENT_REDIR_DOC_RESPONSE";
			break;
		case CgiParser::UNKNOWN_RESPONSE:
			os << "UNKNOWN_RESPONSE";
			break;
		default:
			os << "INVALID_RESPONSE_TYPE";
			break;
	}
	return os;
}

std::vector< char > CgiParser::getBody() const
{
	return _body;
}

void CgiParser::setState(const State &state)
{
	// std::cerr << "CgiParser: Change State " << _state << " -> " << state << std::endl;
	_state = state;
}

// void CgiParser::setResponseType(const ResponseType &responsetype)
// {
// 	std::cerr << "CgiParser: Setting Response Type to " << responsetype << std::endl;
// 	_responseType = responsetype;
// }
// void CgiParser::setStatusCode(const StatusCode &statusCode)
// {
// 	std::cerr << "CgiParser: Setting Status Code to " << statusCode << std::endl;
// 	_statusCode = statusCode;
// }

#include "HttpParser.hpp"
#include "Utils.hpp"
HttpParser::HttpParser(ServerConfig &config) : _state(START_LINE), _error(NO_ERROR), _request(), _start_line_reader(), _header_field_reader(), _body_reader(new BodyReader()), _raw_request(""), _config(config)
{
	std::cout << "HttpParser constructed" << std::endl;
}

HttpParser::~HttpParser()
{
	delete _body_reader;
	std::cout << "HttpParser destructed" << std::endl;
}
HttpParserState HttpParser::getState() const
{
	return _state;
}

void HttpParser::sendData(const char *buf, size_t len)
{
	if (_state == DONE || _state == ERROR)
		return;
	if (len > 0)
		_raw_request.append(buf, len);
	switch (_state)
	{
		case START_LINE:
			getState();
			if (!_start_line_reader.parse(_raw_request, _request, _config))
			{
				if (_start_line_reader.getErrorCode() != 0)
				{
					_state = ERROR;
					_error = (ErrorCode)_start_line_reader.getErrorCode();
					_error_message = _start_line_reader.getErrorMessage();
				}
				break;
			}
			_state = HEADER_FIELDS;
			// fall through
		case HEADER_FIELDS:
			getState();
			if (!(_header_field_reader.parse(_raw_request, _request, _config)))
			{
				if (_header_field_reader.getErrorCode() != 0)
				{
					_state = ERROR;
					_error = (ErrorCode)_header_field_reader.getErrorCode();
					_error_message = _header_field_reader.getErrorMessage();
				}
				break;
			}
			if (_request.getHeader("Transfer-Encoding") == "chunked")
			{
				delete _body_reader;
				_body_reader = new ChunkBodyReader();
			}
			else if (_request.getHeader("Content-Length") != "")
				_body_reader->setLen(std::atoi(_request.getHeader("Content-Length").c_str()));
			else
			{
				_state = DONE;
				break;
			}
			_state = BODY;
			// fall through
		case BODY:
			getState();
			if (!(_body_reader->parse(_raw_request, _request, _config)))
			{
				if (_body_reader->getErrorCode() != 0)
				{
					_state = ERROR;
					_error = (ErrorCode)_body_reader->getErrorCode();
					_error_message = _body_reader->getErrorMessage();
				}
				break;
			}
			_state = DONE;
			// fall through
		case DONE:
			break;
		case ERROR:
			break;
	}
	std::cout << "HttpParser sendData End" << std::endl;
}

HttpResponse HttpParser::getResponse(ServerConfig &config)
{
	HttpResponse res;
	res.setStatusCode(_error);
	res.setStatusMessage(_error_message);
	if (config.error_page.find(_error) != config.error_page.end())
	{
		std::string error_page_path = config.error_page[_error];
		std::fstream file(error_page_path.c_str(), std::ios::binary);
		if (file)
		{
			std::ostringstream ss;
			ss << file.rdbuf();
			res.setBody(ss.str());
			res.setHeader("Content-Type", MimeTypes::getType(error_page_path));
			file.close();
			return res;
		}
	}
	else
	{
		res.setBody(std::string("<html><body><h1>Error ") + to_string(_error) + "</h1><p>" + _error_message + "</p></body></html>");
		res.setHeader("Content-Type", "text/html");
	}
	return res;
}

std::ostream &operator<<(std::ostream &os, const HttpParserState &state)
{
	switch (state)
	{
		case START_LINE:
			os << "START_LINE";
			break;
		case HEADER_FIELDS:
			os << "HEADER_FIELDS";
			break;
		case BODY:
			os << "BODY";
			break;
		case DONE:
			os << "DONE";
			break;
		case ERROR:
			os << "ERROR";
			break;
	}
	return os;
}
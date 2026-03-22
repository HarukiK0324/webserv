#include "../include/StartLineReader.hpp"
#include "../include/ServerConfig.hpp"
#include <iostream>


StartLineReader::StartLineReader() : _buffer("") {}

StartLineReader::~StartLineReader() {}

bool StartLineReader::parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config)
{
    size_t pos = raw_request.find(CRLF);
    if(pos == std::string::npos)
    {
            std::cerr << "CRLF not found in start line" << std::endl;
            setError(BAD_REQUEST, "CRLF not found in start line");
            return false;
    }
    _buffer = raw_request.substr(0, pos);
    size_t first_sp = _buffer.find(" ");
    if(first_sp == std::string::npos)
    {
        setError(BAD_REQUEST, "missing space in start line");
        return false;
    }
    request.setMethod(_buffer.substr(0, first_sp));
    size_t second_sp = _buffer.find(" ", first_sp + 1);
    if(second_sp == std::string::npos)
    {
        setError(BAD_REQUEST, "missing second space in start line");
        return false;
    }
    request.setPath(_buffer.substr(first_sp + 1, second_sp - first_sp - 1));
    request.setProtocol(_buffer.substr(second_sp + 1));
    if(request.getProtocol() != "HTTP/1.1" && request.getProtocol() != "HTTP/1.0")
    {
		setError(HTTP_VERSION_NOT_SUPPORTED, "HTTP version not supported");
		return false;
    }
    raw_request.erase(0, pos + 2);
    std::string start_line = request.getMethod() + " " + request.getPath() + " " + request.getProtocol();
    if(start_line.size() > config.start_line_max_size)
    {
        setError(URI_TOO_LONG, "Start line too long");
        return false;
    }
    return true;
}
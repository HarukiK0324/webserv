#include "../include/HttpResponse.hpp"
#include "../include/Utils.hpp"
HttpResponse::HttpResponse() : _version("HTTP/1.1"), _statusCode(200), _statusMessage("OK"), _headers(), _body("") {}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatusCode(int code)
{
    _statusCode = code;
    switch(code)
    {
        case 200: _statusMessage = "OK"; break;
        case 201: _statusMessage = "Created"; break;
        case 204: _statusMessage = "No Content"; break;
        case 301: _statusMessage = "Moved Permanently"; break;
        case 302: _statusMessage = "Found"; break;
        case 400: _statusMessage = "Bad Request"; break;
        case 403: _statusMessage = "Forbidden"; break;
        case 404: _statusMessage = "Not Found"; break;
        case 405: _statusMessage = "Method Not Allowed"; break;
        case 408: _statusMessage = "Request Timeout"; break;
        case 413: _statusMessage = "Payload Too Large"; break;
        case 414: _statusMessage = "URI Too Long"; break;
        case 431: _statusMessage = "Header Fields Too Large"; break;
        case 500: _statusMessage = "Internal Server Error"; break;
        case 501: _statusMessage = "Not Implemented"; break;
        case 505: _statusMessage = "HTTP Version Not Supported"; break;
        default: _statusMessage = "Unknown Status"; break;
    }
}

void HttpResponse::setErrorBody(const std::string& message)
{
    if(message.empty())
    {
        std::string error_message = _statusMessage.empty() ? "Unknown Error" : _statusMessage;
        setStatusCode(_statusCode); // _statusMessageをリセット
        _body = "<html>\n"
                "<head>\n"
                "<title>" + to_string(_statusCode) + " " + _statusMessage + "</title>\n"
                "</head>\n"
                "<body>\n"
                "<h1>" + error_message + "</h1>\n"
                "</body>\n"
                "</html>";
    }
    else
    {
    _body = "<html>\n"
            "<head>\n"
            "<title>" + message + "</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>" + message + "</h1>\n"
            "</body>\n"
            "</html>";
    }
}

std::string HttpResponse::toString() const
{
    std::ostringstream res;

    res << _version << " " << _statusCode << " " << _statusMessage << CRLF;

    for(std::map<std::string, std::string>::const_iterator it = _headers.begin();it != _headers.end();++it)
    {
        res << it->first << ": " << it->second << CRLF;
    }

    if(_headers.find("Date") == _headers.end())
    {
        time_t now = time(0);
        char buf[100];
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
        res << "Date: " << buf << CRLF;
    }
    if(!_body.empty())
    {
        res << "Content-Length: " << _body.size() << CRLF;
        res << CRLF;
        res << _body;
    }
    else
        res << CRLF;
    return res.str();
}
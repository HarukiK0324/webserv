#include "HeaderFieldReader.hpp"
#include <iostream>
HeaderFieldReader::HeaderFieldReader() : _buffer(""), _header_size(0) {}

HeaderFieldReader::~HeaderFieldReader() {}

bool cookieParser(const std::string& cookie_header, HttpRequest& request) {
    std::istringstream stream(cookie_header);
    std::string cookie_pair;

    if(cookie_header.empty())
        return true;
    while (std::getline(stream, cookie_pair, ';')) {
        size_t eq_pos = cookie_pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = cookie_pair.substr(0, eq_pos);
            std::string value = cookie_pair.substr(eq_pos + 1);
            key.erase(0, key.find_first_not_of(" "));
            key.erase(key.find_last_not_of(" ") + 1);
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);
            if (!key.empty())
                request.addCookie(key, value);
        }
    }
    return true;
}

bool HeaderFieldReader::parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config)
{
    size_t pos;
    while (true)
    {
        pos = raw_request.find(CRLF);
        if (pos == std::string::npos)
            return false;
        if (pos == 0)
        {
            raw_request.erase(0, 2);
            break;
        }
        std::string header = raw_request.substr(0, pos);
        size_t colon_pos = header.find(COLON);
        if(colon_pos == std::string::npos)
        {
            setError(BAD_REQUEST, "Header field missing colon");
            return false;
        }
        std::string key = header.substr(0, colon_pos);
        if(key.empty())
        {
            setError(BAD_REQUEST, "Header field missing key");
            return false;
        }
        std::string value = header.substr(colon_pos + 1);
        size_t first_non_space = value.find_first_not_of(" ");
        if(first_non_space == std::string::npos)
        {
            value = ""; //error?
            setError(BAD_REQUEST, "Header field missing value");
            return false;
        }
        value = value.substr(first_non_space);
        size_t last_non_space = value.find_last_not_of(" ");
        if(last_non_space != std::string::npos)
            value = value.substr(0, last_non_space + 1);
        request.setHeader(key, value);
        raw_request.erase(0, pos + 2);
        _header_size += pos + 2;
    }
    if(_header_size > config.header_max_size)
    {
        setError(HEADER_TOO_LARGE, "Header fields too large");
        return false;
    }
    if(!cookieParser(request.getHeader("Cookie"), request))
    {
        setError(BAD_REQUEST, "Invalid Cookie header");
        return false;
    }
    return true;
}

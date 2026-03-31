#include "HttpRequest.hpp"
#include <cctype>

std::string HttpRequest::normalizeHeaderKey(const std::string& key)
{
    std::string lower_key = key;
    for (size_t i = 0; i < lower_key.size(); ++i)
        lower_key[i] = static_cast<char>(tolower(static_cast<unsigned char>(lower_key[i])));
    return lower_key;
}

HttpRequest::HttpRequest() : method(""), path(""), protocol(""), _header_fields(), _body(""), cookies() {}

HttpRequest::~HttpRequest() {}

std::string HttpRequest::getMethod() const
{
    return method;
}

std::string HttpRequest::getPath() const
{
    return path;
}

std::string HttpRequest::getProtocol() const
{
    return protocol;
}

std::string HttpRequest::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = _header_fields.find(normalizeHeaderKey(key));
    if(it != _header_fields.end())
        return it->second;
    return "";
}

std::string HttpRequest::getBody() const
{
    return _body;
}

std::string HttpRequest::getQueryString() const
{
    return query_string;
}

std::string HttpRequest::getCookie(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = cookies.find(key);
    if(it != cookies.end())
        return it->second;
    return "";
}

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include <cctype>

class HttpRequest
{
    public:
        HttpRequest();
        ~HttpRequest();

        std::string getHeader(const std::string& key) const;
        std::string getMethod() const;
        std::string getPath() const;
        std::string getProtocol() const;
        std::string getQueryString() const;
        std::string getBody() const;

        void setMethod(const std::string& m) { method = m; }
        void setPath(const std::string& p) { path = p; }
        void setProtocol(const std::string& p) { protocol = p; }
        void setHeader(const std::string& key, const std::string& value) {
            _header_fields[normalizeHeaderKey(key)] = value;
        }
        void setBody(const std::string& b) { _body = b; }
        void addBody(const std::string& b) { _body += b; }
        void addCookie(const std::string& key, const std::string& value) { cookies[key] = value; }
        std::string getCookie(const std::string& key) const;
        void setSessionId(const std::string& id) { session_id = id; }
        std::map<std::string, std::string> getCookies() const { return cookies; }
    private:
        static std::string normalizeHeaderKey(const std::string& key);
        std::string method;
        std::string path;
        std::string protocol;
        std::map<std::string, std::string> _header_fields;
        std::string _body;
        std::map<std::string, std::string> cookies;
        std::string session_id;
};

#endif
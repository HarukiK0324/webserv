#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#define CRLF "\r\n"

/*
HTTP-message   = start-line
                 *( header-field CRLF )
                 CRLF
                 [ message-body ]
*/
class HttpResponse
{
    private:
        std::string _version;
        int _statusCode;
        std::string _statusMessage;
        std::map<std::string, std::string> _headers;
        std::string _body;
    //start-line = status-line
    //status-line = HTTP-version SP status-code SP reason-phrase CRLF
    //header-field   = field-name ":" OWS field-value OWS
    //messaage-body  = *OCTET
    public:
        HttpResponse();
        ~HttpResponse();
        void setStatusCode(int code);
        void setStatusMessage(const std::string& message) { _statusMessage = message; }
        void setHeader(const std::string& key, const std::string& value) { _headers[key] = value; }
        void setBody(const std::string& body) { _body = body; }
        void addBody(const std::string& body) { _body += body; }
        void setErrorBody(const std::string& message);
        std::string toString() const;
        int getStatusCode() const { return _statusCode; };
};

#endif
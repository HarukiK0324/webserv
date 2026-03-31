#ifndef READER_HPP
#define READER_HPP

#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "HttpErrorCode.hpp"
#include <string>
#define CRLF "\r\n"
#define COLON ":"

class Reader
{
    public:
        Reader() : _error_code(0), _error_msg("") {}
        virtual ~Reader() {}
        int getErrorCode() const { return _error_code; }
        std::string getErrorMessage() const { return _error_msg; }
        int getErrorCode() { return _error_code; }
        std::string getErrorMessage() { return _error_msg; }
        void setError(int code, const std::string& message)
        {
            _error_code = code;
            _error_msg = message;
        }
        virtual bool parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config) = 0;
    private:
        int _error_code;
        std::string _error_msg;
};

#endif
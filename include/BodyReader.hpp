#ifndef BODYREADER_HPP
#define BODYREADER_HPP

#include <string>
#include <cstdlib>
#include "Reader.hpp"

class BodyReader : public Reader
{
    public:
        BodyReader() : Reader(),  _to_read(0), _read_bytes(0) {}
        ~BodyReader() {}

        void setLen(size_t len) { _to_read = len; }
        virtual bool parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config);
    protected:
        size_t      _to_read;
        size_t      _read_bytes;
};

#endif
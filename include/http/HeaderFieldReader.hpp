#ifndef HEADERFIELDREADER_HPP
#define HEADERFIELDREADER_HPP

#include <sstream>
#include <string>
#include <map>
#include "Reader.hpp"

class HeaderFieldReader : public Reader
{
    private:
        std::string _buffer;
        size_t _header_size;

    public:
		HeaderFieldReader();
		~HeaderFieldReader();
		bool parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config);
};

#endif
#ifndef STARTLINEREADER_HPP
#define STARTLINEREADER_HPP

#include <string>
#include <vector>
#include "Reader.hpp"

class StartLineReader : public Reader
{
    private:
        std::string _buffer;

    public:
		StartLineReader() ;
		~StartLineReader();
		bool parse(std::string &raw_request, HttpRequest &request, const ServerConfig &config);
};

#endif
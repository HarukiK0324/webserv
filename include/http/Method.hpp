#ifndef METHOD_HPP
#define METHOD_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "Utils.hpp"

#include <cstdlib>
#include <climits>

class Method
{
    public:
        virtual ~Method() {}
        virtual void execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config) = 0;
    protected:
        std::string buildPath(const std::string req, const std::string loc, std::string loc_root);
        std::string findLocation(const HttpRequest& req, const ServerConfig& config);
        bool checkPath(const std::string& path);
};

#endif

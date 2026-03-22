#ifndef GETMETHOD_HPP
#define GETMETHOD_HPP

#include "Method.hpp"
#include "MimeTypes.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

class GetMethod : public Method
{
    public:
        void execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config);
};

#endif
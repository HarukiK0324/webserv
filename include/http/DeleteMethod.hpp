#ifndef DELETEMETHOD_HPP
#define DELETEMETHOD_HPP

#include "Method.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

class DeleteMethod : public Method
{
    public:
        void execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config);
};

#endif
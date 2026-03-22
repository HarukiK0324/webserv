#ifndef POSTMETHOD_HPP
#define POSTMETHOD_HPP

#include "Method.hpp"
#include <fstream>
#include <sstream>
#include <map>


struct MultiPartData {
    std::map<std::string, std::string> headers;
    std::string name;
    std::string filename;
    std::string data;
    bool is_file;
};

class PostMethod : public Method
{
    private:
        std::map<std::string, std::string> parseUrlEncodedForm(const std::string& body);
        std::vector<MultiPartData> parseMultipartBody(const std::string& body, const std::string& boundary);
    public:
        void execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config);
};
#endif
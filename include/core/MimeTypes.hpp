#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <map>
#include <string>

class MimeTypes
{
    public:
        static std::string getType(const std::string& extension);
};

#endif
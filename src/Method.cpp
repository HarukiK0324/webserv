#include "Method.hpp"


std::string Method::buildPath(const std::string req, const std::string loc, std::string loc_root)
{
    std::string req_path = req.substr(loc.length());
    if (!req_path.empty() && req_path[0] != '/')
        req_path = "/" + req_path;
    if(!loc_root.empty() && loc_root.back() == '/')
        loc_root = loc_root.substr(0, loc_root.length() - 1);
    if(req_path.empty() || req_path == "/")
        return loc_root;
    else
        return loc_root + req_path;
}

std::string findLocation(const HttpRequest& req, const ServerConfig& config)
{
    std::string req_path = req.getPath();
    std::string best_match = "";
    for(std::map<std::string, LocationConfig>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it)
    {
        const std::string& location_path = it->first;
        if(req_path.find(location_path) == 0)
        {
            if(location_path.length() > best_match.length())
                best_match = location_path;
        }
    }
    return best_match;
}

bool checkPath(const std::string& path)
{
    char resolvedRoot[PATH_MAX];
    char resolvedFull[PATH_MAX];

    if(realpath(path.c_str(), resolvedRoot) == NULL)
        return false;
}
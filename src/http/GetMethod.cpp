#include "GetMethod.hpp"

void GetMethod::execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config)
{
    // 1. Construct the absolute file path (Root + URI)
    // 2. Check if file exists (stat/access)
    // 3. If directory -> handle Autoindex
    // 4. If file -> Open, Read content, Set Body, Set 200 OK
    // 5. If missing -> Set 404 Not Found
    std::string full_path;
    if(!resolvePathUnderRoot(config.root, req.getPath(), full_path))
    {
        res.setStatusCode(400);
        res.setErrorBody("400 Bad Request: Invalid path");
        res.setHeader("Content-Type", "text/plain");
        return;
    }
    std::cerr << "GetMethod: Requested path = " << full_path << std::endl;
    struct stat file_stat;
    if(stat(full_path.c_str(), &file_stat) == -1)
    {
        res.setStatusCode(404);
        res.setErrorBody("404 Not Found");
        res.setHeader("Content-Type", "text/plain");
        return;
    }
    else if(S_ISDIR(file_stat.st_mode))
    {
        if(!config.index.empty())
        {
            for(size_t i = 0; i < config.index.size(); ++i)
            {
                std::string index_file = config.index[i];
                std::string index_path = (index_file[0] == '/') ? full_path + index_file : full_path + "/" + index_file;
                if(stat(index_path.c_str(), &file_stat) != -1 && S_ISREG(file_stat.st_mode))
                {
                    full_path = index_path;
                    break;
                }
            }
        }
        else
        {
            if(config.autoindex)
            {
                DIR *dir = opendir(full_path.c_str());
                if(!dir)
                {
                    res.setStatusCode(500);
                    res.setErrorBody("500 Internal Server Error");
                    res.setHeader("Content-Type", "text/plain");
                    return;
                }
                while(struct dirent *entry = readdir(dir))
                {
                    std::string entry_name = entry->d_name;
                    if(entry_name == "." || entry_name == "..")
                        continue;
                    res.addBody("Index: " + entry_name + "\n");
                }
                res.setStatusCode(200);
                res.setHeader("Content-Type", "text/plain");
                closedir(dir);
                return;
            }
            else
            {
                res.setStatusCode(403);
                res.setErrorBody("403 Forbidden");
                res.setHeader("Content-Type", "text/plain");
                return;
            }
        }
    }
    std::cerr << "GetMethod: Serving file " << full_path << std::endl;
    if(access(full_path.c_str(), F_OK | R_OK) == 0)
    {
		std::ifstream file(full_path.c_str(), std::ios::in | std::ios::binary);
		if(!file)
        {
            res.setStatusCode(500);
            res.setErrorBody("500 Internal Server Error");
            res.setHeader("Content-Type", "text/plain");
            return;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        res.setStatusCode(200);
        res.setBody(ss.str());
        res.setHeader("Content-Type", MimeTypes::getType(full_path));
        file.close();
    }
    else
    {
        res.setStatusCode(403);
        res.setErrorBody("403 Forbidden");
        res.setHeader("Content-Type", "text/plain");
        return;
    }
}

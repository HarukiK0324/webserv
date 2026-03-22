#include "../include/DeleteMethod.hpp"

void DeleteMethod::execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config)
{
    std::string full_path = config.root + req.getPath();
    struct stat file_stat;
    if(stat(full_path.c_str(), &file_stat) == -1)
    {
        res.setStatusCode(404);
        res.setErrorBody("404 Not Found");
        return;
    }
    else if(S_ISDIR(file_stat.st_mode))
    {
        res.setStatusCode(403);
        res.setErrorBody("403 Forbidden: Cannot delete a directory");
        return;
    }
    if(access(full_path.c_str(), W_OK) == -1)
    {
        res.setStatusCode(403);
        res.setErrorBody("403 Forbidden: Permission denied");
        return;
    }
    std::cerr << "Deleting file: " << full_path << std::endl;
    if(remove(full_path.c_str()) != 0)
    {
        res.setStatusCode(500);
        res.setErrorBody("500 Internal Server Error");
        return;
    }
    res.setStatusCode(204);
    res.setBody("");
    // 1. Construct path
    // 2. Check existence
    // 3. delete() or remove() the file
    // 4. Set 204 No Content (or 200 OK)
}
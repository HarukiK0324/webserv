#include "../include/MimeTypes.hpp"

std::string MimeTypes::getType(const std::string &path)
{
	static std::map< std::string, std::string > types;
	if (types.empty())
	{
		types["html"] = "text/html";
		types["htm"] = "text/html";
		types["css"] = "text/css";
		types["js"] = "application/javascript";
		types["json"] = "application/json";
		types["png"] = "image/png";
		types["jpg"] = "image/jpeg";
		types["jpeg"] = "image/jpeg";
		types["gif"] = "image/gif";
		types["txt"] = "text/plain";
		types["pdf"] = "application/pdf";
	}
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "application/octet-stream";
	std::string extention = path.substr(pos + 1);
	std::map< std::string, std::string >::iterator it = types.find(extention);
	if (it != types.end())
		return it->second;
	return "application/octet-stream";
}
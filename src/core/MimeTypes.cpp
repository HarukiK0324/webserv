#include "MimeTypes.hpp"
#include <cctype>

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
		types["zip"] = "application/zip";
		types["mp4"] = "video/mp4";
		types["avi"] = "video/x-msvideo";
		types["mpeg"] = "video/mpeg";
		types["mp3"] = "audio/mpeg";
		types["wav"] = "audio/wav";
		types["ico"] = "image/x-icon";
		types["svg"] = "image/svg+xml";
		types["xml"] = "application/xml";
		types["csv"] = "text/csv";
		types["webp"] = "image/webp";
		types["bmp"] = "image/bmp";
		types["tiff"] = "image/tiff";
		types["woff"] = "font/woff";
		types["woff2"] = "font/woff2";
		types["eot"] = "application/vnd.ms-fontobject";
		types["otf"] = "font/otf";
		types["7z"] = "application/x-7z-compressed";
		types["rar"] = "application/x-rar-compressed";
		types["tar"] = "application/x-tar";
		types["gz"] = "application/gzip";
		types["bz2"] = "application/x-bzip2";
		types["xz"] = "application/x-xz";
		types["sh"] = "application/x-sh";
		types["c"] = "text/x-c";
		types["cpp"] = "text/x-c++";
		types["h"] = "text/x-c";
		types["hpp"] = "text/x-c++";
	}
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "application/octet-stream";
	std::string extention = path.substr(pos + 1);
	for (size_t i = 0; i < extention.size(); ++i)
		extention[i] = static_cast< char >(std::tolower(static_cast< unsigned char >(extention[i])));
	std::map< std::string, std::string >::iterator it = types.find(extention);
	if (it != types.end())
		return it->second;
	return "application/octet-stream";
}
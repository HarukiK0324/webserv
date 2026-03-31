
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "Config.hpp"
#include "Utils.hpp"

static bool isHexDigit(char c)
{
	return (c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'f')
		|| (c >= 'A' && c <= 'F');
}

static int hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return c - 'A' + 10;
}

void safty_close(int &fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

bool decodePercentEncoded(const std::string &input, std::string &output)
{
	output.clear();
	output.reserve(input.size());
	for (size_t i = 0; i < input.size(); ++i)
	{
		if (input[i] == '%')
		{
			if (i + 2 >= input.size() || !isHexDigit(input[i + 1]) || !isHexDigit(input[i + 2]))
				return false;
			char c = static_cast<char>((hexValue(input[i + 1]) << 4) | hexValue(input[i + 2]));
			if (c == '\0')
				return false;
			output.push_back(c);
			i += 2;
		}
		else
			output.push_back(input[i]);
	}
	return true;
}

bool normalizeRequestPath(const std::string &requestTarget, std::string &normalizedPath)
{
	if (requestTarget.empty() || requestTarget[0] != '/')
		return false;

	std::string pathOnly = requestTarget;
	size_t qpos = pathOnly.find('?');
	if (qpos != std::string::npos)
		pathOnly = pathOnly.substr(0, qpos);

	// Reject percent-encoded path separators to keep traversal checks simple.
	for (size_t i = 0; i + 2 < pathOnly.size(); ++i)
	{
		if (pathOnly[i] == '%' && isHexDigit(pathOnly[i + 1]) && isHexDigit(pathOnly[i + 2]))
		{
			char decoded = static_cast<char>((hexValue(pathOnly[i + 1]) << 4) | hexValue(pathOnly[i + 2]));
			if (decoded == '/' || decoded == '\\')
				return false;
		}
	}
	std::string decodedPath;
	if (!decodePercentEncoded(pathOnly, decodedPath))
		return false;

	std::vector< std::string > safeParts;
	size_t start = 0;
	while (start <= decodedPath.size())
	{
		size_t end = decodedPath.find('/', start);
		if (end == std::string::npos)
			end = decodedPath.size();
		std::string part = decodedPath.substr(start, end - start);

		if (part.empty() || part == ".")
		{
			// Skip empty segments (from leading/trailing/multiple slashes) and current-directory references.
		}
		else if (part == "..")
		{
			// Attempt to go above the root: reject.
			if (safeParts.empty())
				return false;
			safeParts.pop_back();
		}
		else
		{
			safeParts.push_back(part);
		}
		if (end == decodedPath.size())
			break;
		start = end + 1;
	}
	normalizedPath = "/";
	for (size_t i = 0; i < safeParts.size(); ++i)
	{
		normalizedPath += safeParts[i];
		if (i + 1 < safeParts.size())
			normalizedPath += "/";
	}
	return true;
}

bool resolvePathUnderRoot(const std::string &rootDir, const std::string &requestPath, std::string &resolvedPath)
{
	std::string normalizedPath;
	if (!normalizeRequestPath(requestPath, normalizedPath))
		return false;
	std::string cleanRoot = rootDir;
	while (cleanRoot.size() > 1 && cleanRoot[cleanRoot.size() - 1] == '/')
		cleanRoot.erase(cleanRoot.size() - 1);
	if (cleanRoot.empty())
		cleanRoot = "/";

	if (cleanRoot == "/")
		resolvedPath = normalizedPath;
	else
		resolvedPath = cleanRoot + normalizedPath;
	return true;
}

std::string to_string(int n)  // void* ではなく int にする
{
	std::stringstream oss;
	oss << n;
	return oss.str();
}

void print_location(const LocationConfig &lc)
{
	std::cout << "    Location \"" << lc.path << "\"\n";
	std::cout << "      root: " << lc.root << "\n";

	// index
	std::cout << "      index: [";
	for (size_t i = 0; i < lc.index.size(); ++i)
	{
		std::cout << lc.index[i];
		if (i + 1 != lc.index.size())
			std::cout << ", ";
	}
	std::cout << "]\n";

	// allow_methods
	std::cout << "      allow_methods: [";
	for (size_t i = 0; i < lc.allow_methods.size(); ++i)
	{
		std::cout << lc.allow_methods[i];
		if (i + 1 != lc.allow_methods.size())
			std::cout << ", ";
	}
	std::cout << "]\n";

	// cgi
	if (!lc.CgiExecuter.empty() || !lc.cgi_type.empty())
	{
		std::cout << "      cgi:\n";
		if (!lc.cgi_type.empty())
			std::cout << "        extension: " << lc.cgi_type << "\n";
		if (!lc.CgiExecuter.empty())
			std::cout << "        executor: " << lc.CgiExecuter << "\n";
	}

	// upload_path
	if (!lc.upload_path.empty())
		std::cout << "      upload_path: " << lc.upload_path << "\n";

	// redirect
	if (!lc.redirect)
	{
		std::cout << "      redirect pages:\n";
		std::cout << "        code: " << lc.redirect_code << "\n";
		std::cout << "        target: " << lc.redirect_target << "\n";
	}
}

void print_server(const std::vector< ServerConfig > &svec)
{
	for (std::vector< ServerConfig >::const_iterator sc = svec.begin(); sc != svec.end(); ++sc)
	{
		std::cout << "ServerConfig\n";
		std::cout << "  listen: " << sc->listen_host << ":" << sc->listen_port
				  << "  (parsed port=" << sc->port << ")\n";

		std::cout << "  root: " << sc->root << "\n";

		std::cout << "  client_max_body_size: ";
		if (sc->client_max_body_size == 0)
			std::cout << "unlimited\n";
		else
			std::cout << sc->client_max_body_size << " bytes\n";

		std::cout << "  keepalive_timeout: " << sc->keepalive_timeout << "s\n";

		// server_name
		std::cout << "  server_name: [";
		for (size_t i = 0; i < sc->server_name.size(); ++i)
		{
			std::cout << sc->server_name[i];
			if (i + 1 != sc->server_name.size())
				std::cout << ", ";
		}
		std::cout << "]\n";

		// index
		std::cout << "  index: [";
		for (size_t i = 0; i < sc->index.size(); ++i)
		{
			std::cout << sc->index[i];
			if (i + 1 != sc->index.size())
				std::cout << ", ";
		}
		std::cout << "]\n";

		// error_page
		if (!sc->error_page.empty())
		{
			std::cout << "  error_page: {\n";
			for (std::map< int, std::string >::const_iterator it = sc->error_page.begin();
				it != sc->error_page.end();
				++it)
			{
				std::cout << "    " << it->first << ": " << it->second << "\n";
			}
			std::cout << "  }\n";
		}

		// locations
		std::cout << "  locations:\n";
		for (std::map< std::string, LocationConfig >::const_iterator it = sc->locations.begin();
			it != sc->locations.end();
			++it)
		{
			print_location(it->second);	 // ここは OK（it->second は値への参照）
		}
	}
}

#include "../include/LocationConfig.hpp"
#include "../include/ServerConfig.hpp"
#include "../include/Config.hpp"
#include "../include/Utils.hpp"

void safty_close(int &fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
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
#include "ServerConfig.hpp"
#include "ConfigParser.hpp"
#include "ConfigBuilder.hpp"
#include "LocationConfig.hpp"

#include "Config.hpp"
#include "Utils.hpp"
#include "PortManager.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	try
	{
		(void)argc;
		ConfigParser a(argv[1]);
		a.lexer();
		a.printTokens();
		a.parser();
		ConfigBuilder confs(a.getAst());
		std::map< ListenKey, std::vector< ServerConfig > > sconfs = confs.buildAll();
		for (std::map< ListenKey, std::vector< ServerConfig > >::const_iterator it = sconfs.begin(); it != sconfs.end(); ++it)
		{
			print_server(it->second);
			std::cout << "----------------------------------------\n";
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
}
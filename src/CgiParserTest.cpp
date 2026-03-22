#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "../include/CgiHandler.hpp"
#include "../include/Utils.hpp"
#include "../include/CgiParser.hpp"
#include "../include/PortManager.hpp"
#include "../include/ServerLoop.hpp"

#include "../include/ConfigParser.hpp"
#include "../include/ConfigBuilder.hpp"

// static void setup_request_data(std::string startline, std::map< std::string, std::string > headers, std::string body)
// {
// 	startline = "POST /cgi-bin/script.py/extra/path?name=gemini&id=123 HTTP/1.1";
// 	headers["Host"] = "localhost:8080";
// 	headers["User-Agent"] = "Mozilla/5.0 (CGI-Test-Client)";
// 	headers["Content-Type"] = "application/x-www-form-urlencoded";
// 	headers["Content-Length"] = "25";
// 	headers["X-Custom-Header"] = "Debug-Value";
// 	body = "field1=value1&field2=test";
// 	//環境変数の設定
// }

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}
	ConfigParser parser(argv[argc - 1]);
	parser.lexer();
	parser.parser();
	ConfigBuilder buidlder(parser.getAst());
	std::map< ListenKey, std::vector< ServerConfig > > sconfs = buidlder.buildAll();
	PortManager pm = PortManager(sconfs);
	ServerLoop loop = ServerLoop(pm.getPortfdAndConfs());
	ServerConfig sconf = sconfs.begin()->second[0];
	std::string client_ip = "127.0.0.1";
	CgiHandler *Ch = new CgiHandler(sconf.locations["cgi-bin"], &loop);
	std::string startline = "POST /cgi-bin/script.py/extra/path?name=gemini&id=123 HTTP/1.1";
	std::string body = "field1=value1&field2=test";
	std::string port = "8080";
	std::map< std::string, std::string > headers;
	headers["Host"] = "localhost";
	headers["User-Agent"] = "Mozilla/5.0 (CGI-Test-Client)";
	headers["Content-Type"] = "application/x-www-form-urlencoded";
	headers["Content-Length"] = "25";
	headers["X-Custom-Header"] = "Debug-Value";
	Ch->MakeEnvp(startline, headers, body, client_ip, port);
	Ch->PrintEnvp();  // 代入式ではなく、この	書き方が一般的です
	return 0;
}

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP
#include <string>
#include <vector>
#include <map>
#include "Config.hpp"
#include "LocationConfig.hpp"
#define START_LINE_MAX_SIZE 7168 //7kB
#define HTTP_PARSER_SIZE 100000

//複数回の定義を許すか、許さないかで、flagの有無が変わる
struct ServerConfig
{
	int port;								  //ListenHandler
	std::string listen_host, listen_port;	  //ListenHandler
	std::vector< std::string > server_name;	  //Host headerと一致
	size_t client_max_body_size;			  //httpParser
	size_t start_line_max_size;				  //httpParser
	size_t header_max_size;					  //httpParser
	std::map< int, std::string > error_page;  //httpProtocol,httpResponse

	size_t keepalive_timeout;  // ConnectionHandlerのタイムアウト時間
	//locationへ継承されるもの
	std::string root;
	std::vector< std::string > allow_methods;
	std::vector< std::string > index;
	bool autoindex;
	std::map< std::string, LocationConfig > locations;
	ServerConfig() :
		port(8000),
		listen_host("*"),
		listen_port("8000"),
		client_max_body_size(HTTP_PARSER_SIZE),
		start_line_max_size(START_LINE_MAX_SIZE),
		header_max_size(HTTP_PARSER_SIZE),
		keepalive_timeout(75),
		root("html"),
		autoindex(false) {};
};

//	keepalive_timeout(75),
#endif

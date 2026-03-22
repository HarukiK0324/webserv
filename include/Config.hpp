
#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <vector>
#include <map>

struct ListenKey
{
	int port;
	std::string host;
	bool operator<(const ListenKey &o) const
	{
		if (port != o.port)
			return port < o.port;
		return host < o.host;
	}
};
struct Directive
{
	std::string key;
	std::vector< std::string > values;
};

struct LocationNode
{
	std::string path;
	std::vector< Directive > directives;
};

struct ServerNode
{
	std::vector< Directive > directives;
	std::map< std::string, LocationNode > locations;
};

// ===== メタ文字 =====
#define SERVER "server"
#define LOCATION "location"
#define BRACE_OPEN '{'
#define BRACE_CLOSE '}'
#define SEMICOLON ';'
#define NEWLINE '\n'

#endif
// -----------文法----------
// server_block  := "server" "{" server_directive* location_block* "}"
// location_block:= "location" PATH "{" location_directive* "}"
// server_directive   := KEY VALUE";"
// location_directive := KEY VALUE";"

// reserved words:= server, location
// reserved symbols:= {, }, ; isspace()
// STR:= any string except reserved words and symbols

//課題要件の外、定数での扱いで大丈夫

// struct http_config
// {
// 	// === main コンテキスト相当 ===
// 	// std::string worker_conections_number;  // 同時接続数の上限（1プロセス分）
// 	// std::string include_minefile;		   // mime.types ファイルのパス
// 	// std::string default_type;			   // Content-Type が決まらないときの型
// 	// std::string access_log;				   // アクセスログのパス
// 	// std::string error_log;				   // エラーログのパス
// };
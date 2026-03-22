#include "../include/ConfigBuilder.hpp"
#include <stdint.h>

ConfigBuilder::ConfigBuilder(const std::vector< ServerNode > &ast) :
	_ast(ast)
{
	initLocationDispatch();
	initServerDispatch();
}

ConfigBuilder::~ConfigBuilder()
{
}

std::map< ListenKey, std::vector< ServerConfig > > ConfigBuilder::buildAll()
{
	std::map< ListenKey, std::vector< ServerConfig > > result;
	for (std::vector< ServerNode >::const_iterator it = _ast.begin(); it != _ast.end(); ++it)
	{
		ServerConfig tmp;
		tmp = buildServer(*it);
		ListenKey key;
		key.host = tmp.listen_host;
		key.port = tmp.port;
		if (tmp.server_name.empty())
			tmp.server_name.push_back("");
		result[key].push_back(tmp);
	}
	return result;
}

ServerConfig ConfigBuilder::buildServer(const ServerNode &node)
{
	ServerConfig result;
	for (std::vector< Directive >::const_iterator it = node.directives.begin(); it != node.directives.end(); ++it)
	{
		if (_serverDispatch.count(it->key) == false)
			throw std::runtime_error("Unknown directive in server: " + it->key);
		_serverDispatch[it->key](result, *it);
	}
	if (result.allow_methods.empty())  //default
	{
		result.allow_methods.push_back("GET");
	}
	for (std::map< std::string, LocationNode >::const_iterator it = node.locations.begin(); it != node.locations.end(); ++it)
	{
		LocationConfig lconf;
		lconf = buildLocation(it->second);
		if (!lconf.root_set)
			lconf.root = result.root;
		if (!lconf.index_set)
			lconf.index = result.index;
		if (!lconf.autoindex_set)
			lconf.autoindex = result.autoindex;
		if (!lconf.allow_methods_set)
			lconf.allow_methods = result.allow_methods;
		result.locations[lconf.path] = lconf;
	}
	if (result.locations.find("") == result.locations.end())
	{
		LocationConfig def_loc;
		def_loc.path = "";
		def_loc.root = result.root;
		def_loc.index = result.index;
		def_loc.autoindex = result.autoindex;
		def_loc.allow_methods = result.allow_methods;
		result.locations[""] = def_loc;
	}
	return result;
}

LocationConfig ConfigBuilder::buildLocation(const LocationNode &node)
{
	LocationConfig result;
	result.path = node.path;
	for (std::vector< Directive >::const_iterator it = node.directives.begin(); it != node.directives.end(); ++it)
	{
		if (_locationDispatch.count(it->key) == false)
			throw std::runtime_error("Unknown directive in location: " + it->key);
		_locationDispatch[it->key](result, *it);
	}
	return result;
}

void ConfigBuilder::initServerDispatch()
{
	_serverDispatch["listen"] = &ConfigBuilder::set_listen;
	_serverDispatch["server_name"] = &ConfigBuilder::set_server_name;
	//locとの上書き設定済み
	_serverDispatch["root"] = &ConfigBuilder::set_root;
	_serverDispatch["index"] = &ConfigBuilder::set_index;
	_serverDispatch["autoindex"] = &ConfigBuilder::set_autoindex;
	_serverDispatch["allow_methods"] = &ConfigBuilder::set_allow;
	//serverのみで大丈夫では？
	_serverDispatch["client_max_body_size"] = &ConfigBuilder::set_client_max_body;
	_serverDispatch["error_page"] = &ConfigBuilder::set_error_page;		   //マンダトリーだとここだけで大丈夫
	_serverDispatch["keepalive_timeout"] = &ConfigBuilder::set_keepalive;  // いらないかも？　Serverのみで大丈夫では？
																		   // _serverDispatch["return"] = &ConfigBuilder::set_redirect;  いるのか？
}

void ConfigBuilder::initLocationDispatch()
{
	//上書き対応　Serverでも定義できる
	_locationDispatch["root"] = &ConfigBuilder::loc_set_root;
	_locationDispatch["index"] = &ConfigBuilder::loc_set_index;
	_locationDispatch["autoindex"] = &ConfigBuilder::loc_set_autoindex;
	_locationDispatch["allow_methods"] = &ConfigBuilder::loc_set_allow;
	//location　only 　
	_locationDispatch["cgi_pass"] = &ConfigBuilder::loc_set_cgi;
	_locationDispatch["upload_path"] = &ConfigBuilder::loc_set_upload_path;
	_locationDispatch["return"] = &ConfigBuilder::loc_set_redirect;
}

bool is_digits(const std::string &s)
{
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (!std::isdigit(s[i]))
			return false;
	}
	return !s.empty();
}

// Syntax:	listen address[:port] or add or port
// Default:	listen *:80 | *:8000;
// Context:	server

// listen 127.0.0.1 : 8000;
// listen 127.0.0.1;
// listen 8000;
// listen * : 8000;
// listen localhost : 8000;

//space禁止

// listen[::] : 8000;
// listen[::1];

void ConfigBuilder::set_listen(ServerConfig &sc, const Directive &d)  //だるいね
{
	if (d.values.size() != 1)
		throw std::runtime_error("listen directive requires exactly 1 argument");
	const std::string &val = d.values[0];
	size_t pos = val.find(':');
	if (pos == std::string::npos)
	{
		//port or host 判定必要
		if (is_digits(val))
		{  //port only
			sc.listen_port = val;
			sc.listen_host = "*";
		}
		else
		{
			//host only
			sc.listen_host = val;
			sc.listen_port = "8000";
		}
	}
	else
	{
		// host and port
		sc.listen_port = val.substr(pos + 1);
		sc.listen_host = val.substr(0, pos);
		if (sc.listen_port.empty())
			throw std::runtime_error("listen: port missing");
	}
	if (!is_digits(sc.listen_port))
		throw std::runtime_error("listen: invalid port number");
	sc.port = atoi(sc.listen_port.c_str());
	if (sc.port < 1024)
		throw std::runtime_error("listen: port must be >= 1024 (privileged port)");
}

// Syntax : server_name name...;
// Default : server_name "";
// Context : server

void ConfigBuilder::set_server_name(ServerConfig &sc, const Directive &d)  // ok
{
	for (std::vector< std::string >::const_iterator it = d.values.begin(); it != d.values.end(); ++it)
	{
		sc.server_name.push_back(*it);
	}
}

// Syntax : root path;
// Default : root html;
// Context : http, server, location, if in location

void ConfigBuilder::set_root(ServerConfig &sc, const Directive &d)
{
	if (d.values.size() != 1)
		throw std::runtime_error("root directive expects exactly 1 value");
	sc.root = d.values[0];
}

// Syntax : index file...;
// Default : index index.html;
// Context : http, server, location

void ConfigBuilder::set_index(ServerConfig &sc, const Directive &d)	 // ok
{
	for (std::vector< std::string >::const_iterator it = d.values.begin(); it != d.values.end(); ++it)
		sc.index.push_back(*it);
}

void ConfigBuilder::set_allow(ServerConfig &sc, const Directive &d)
{
	for (std::vector< std::string >::const_iterator it = d.values.begin(); it != d.values.end(); ++it)
	{
		if (!(*it == "GET" || *it == "POST" || *it == "DELETE"))
			throw std::runtime_error(*it + " is not allowed method (only GET POST DELETE)");

		sc.allow_methods.push_back(*it);
	}
}

// Syntax : client_max_body_size size;
// Default : client_max_body_size 1m;
// Context : http, server, location

void ConfigBuilder::set_client_max_body(ServerConfig &sc, const Directive &d)
{
	if (d.values.size() != 1)
		throw std::runtime_error("client_max_body_size: requires exactly 1 argument");
	const std::string &val = d.values[0];
	// サフィックス判定
	char last = val[val.size() - 1];
	bool has_suffix = (last == 'k' || last == 'K' || last == 'm' || last == 'M');
	std::string number_part = has_suffix ? val.substr(0, val.size() - 1) : val;
	if (number_part.empty())
		throw std::runtime_error("client_max_body_size: invalid format");
	// 数字チェック
	for (size_t i = 0; i < number_part.size(); ++i)
	{
		if (!std::isdigit(number_part[i]))
			throw std::runtime_error("client_max_body_size: invalid number: " + val);
	}
	unsigned long long num = std::strtoull(number_part.c_str(), NULL, 10);
	// 0 は無制限という扱い
	if (num == 0)
	{
		sc.client_max_body_size = 0;
		return;
	}
	// Suffix 倍率
	unsigned long long mul = 1;
	if (has_suffix)
	{
		switch (last)
		{
			case 'k':
			case 'K':
				mul = 1024ULL;
				break;
			case 'm':
			case 'M':
				mul = 1024ULL * 1024ULL;
				break;
		}
	}
	// Overflow check
	unsigned long long result = num * mul;
	if (result > SIZE_MAX)
		throw std::runtime_error("client_max_body_size: value too large");
	sc.client_max_body_size = static_cast< size_t >(result);
}

// Syntax:	error_page code ... [=[response]] uri;
// Default:	—
// Context:	http, server, location, if in location
void ConfigBuilder::set_error_page(ServerConfig &sc, const Directive &d)
{
	if (d.values.size() < 2)
		throw std::runtime_error("error_page requires: code... path");
	std::vector< int > codes;
	const std::string &path = d.values.back();

	if (path.empty() || path[0] != '/')	 //これはいるのか？
		throw std::runtime_error("error_page: path must start with '/'");

	for (size_t i = 0; i < d.values.size() - 1; ++i)
	{
		const std::string &token = d.values[i];

		if (!is_digits(token))
			throw std::runtime_error("error_page: invalid status code: " + token);

		int code = std::atoi(token.c_str());
		if (code < 100 || code > 599)
			throw std::runtime_error("error_page: status code out of range: " + token);
		// 必要ならエラーコードの種類を制限（任意）
		codes.push_back(code);
	}
	// すべての code に対して path をセット
	for (size_t i = 0; i < codes.size(); ++i)
		sc.error_page[codes[i]] = path;
}

//ok
void ConfigBuilder::loc_set_root(LocationConfig &lc, const Directive &d)  //ok
{
	if (d.values.size() != 1)
		throw std::runtime_error("root in location requires 1 argument");
	lc.root = d.values[0];
	lc.root_set = true;
}

//ok
void ConfigBuilder::loc_set_index(LocationConfig &lc, const Directive &d)
{
	for (size_t i = 0; i < d.values.size(); ++i)
		lc.index.push_back(d.values[i]);
	lc.index_set = true;
}
//ok　my rule
//Syntax:	allow_methods method . . .:
// Default :
// Context :  location,
void ConfigBuilder::loc_set_allow(LocationConfig &lc, const Directive &d)
{
	for (std::vector< std::string >::const_iterator it = d.values.begin(); it != d.values.end(); ++it)
	{
		if (!(*it == "GET" || *it == "POST" || *it == "DELETE"))
			throw std::runtime_error(*it + " is not allowed method (only GET POST DELETE)");

		lc.allow_methods.push_back(*it);
	}
	lc.allow_methods_set = true;
}
// ok my rule
//Syntax:	cgi_pass .file CgiExecuter
// Default :
// Context : location,
void ConfigBuilder::loc_set_cgi(LocationConfig &lc, const Directive &d)
{
	if (d.values.size() != 2)
		throw std::runtime_error("cgi_pass expects: extension executor");
	if (d.values[0][0] != '.')
		throw std::runtime_error("cgi_pass: extension must start with '.'");
	lc.cgi_type = d.values[0];
	lc.CgiExecuter = d.values[1];
}
//ok my rule
//Syntax:	upload_path path_name
// Default :
// Context : location,
void ConfigBuilder::loc_set_upload_path(LocationConfig &lc, const Directive &d)
{
	if (d.values.size() != 1)
		throw std::runtime_error("upload_path requires 1 argument");
	lc.upload_path = d.values[0];
	lc.upload_enable = true;
}

//return
//Syntax:	return code [text];
// 			return code URL;
// 			return URL;
// Default :
// Context : server, location,
void ConfigBuilder::loc_set_redirect(LocationConfig &lc, const Directive &d)
{
	if (d.values.size() != 2)
		throw std::runtime_error("return requires code and target");
	// ---- status code の確認 ----
	const char *s = d.values[0].c_str();
	for (size_t i = 0; s[i]; ++i)
		if (!isdigit(s[i]))
			throw std::runtime_error("return code must be numeric");
	int code = std::atoi(s);
	if (!(code >= 300 && code < 400))
		throw std::runtime_error("invalid HTTP status code");
	const std::string &target = d.values[1];
	// ---- LocationConfig に設定 ----
	lc.redirect = true;
	lc.redirect_code = code;
	lc.redirect_target = target;
}

// Syntax  : autoindex on | off;
// Default : autoindex off;
// Context : http, server, location

void ConfigBuilder::set_autoindex(ServerConfig &sc, const Directive &d)
{
	if (d.values.size() != 1)
		throw std::runtime_error("autoindex requires exactly one argument");

	const std::string &val = d.values[0];

	if (val == "on")
		sc.autoindex = true;
	else if (val == "off")
		sc.autoindex = false;
	else
		throw std::runtime_error("autoindex must be 'on' or 'off'");
}

void ConfigBuilder::loc_set_autoindex(LocationConfig &lc, const Directive &d)
{
	if (d.values.size() != 1)
		throw std::runtime_error("autoindex requires exactly one argument");

	const std::string &val = d.values[0];

	if (val == "on")
		lc.autoindex = true;
	else if (val == "off")
		lc.autoindex = false;
	else
		throw std::runtime_error("autoindex must be 'on' or 'off'");
	lc.autoindex_set = true;
}

// // Syntax  : upload_enable on | off
// // Default : upload_enable off
// // Context : location

// void ConfigBuilder::loc_set_upload_enable(LocationConfig &lc, const Directive &d)
// {
// 	if (d.values.size() != 1)
// 		throw std::runtime_error("upload_enable requires exactly one argument");

// 	const std::string &val = d.values[0];

// 	if (val == "on")
// 		lc.upload_enable = true;
// 	else if (val == "off")
// 		lc.upload_enable = false;
// 	else
// 		throw std::runtime_error("upload_enable must be 'on' or 'off'");
// }

// Syntax : keepalive_timeout ;
// Default : keepalive_timeout 75s;
// Context : http, server, location　　serverのみで大丈夫ではないか？

void ConfigBuilder::set_keepalive(ServerConfig &sc, const Directive &d)	 //単位など
{
	if (d.values.size() != 1)
		throw std::runtime_error("keepalive_timeout expects 1 value");
	const std::string &val = d.values[0];
	char last = val[val.size() - 1];
	bool time = (last == 's' || last == 'S');
	std::string num = time ? val.substr(0, val.size() - 1) : val;
	if (num.empty())
		throw std::runtime_error("keepalive_timeout: invalid format");
	for (size_t i = 0; i < num.size(); ++i)
	{
		if (!std::isdigit(num[i]))
			throw std::runtime_error("keepalive: invalid number: " + val);
	}
	int timeout = std::atoi(num.c_str());
	sc.keepalive_timeout = timeout;
}
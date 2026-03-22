#ifndef CONFIGBUILDER_HPP
#define CONFIGBUILDER_HPP
#include <map>
#include <vector>
#include <string>
#include "Config.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class ConfigBuilder
{
public:
	typedef void (*ServerSetter)(ServerConfig &, const Directive &);
	typedef void (*LocationSetter)(LocationConfig &, const Directive &);

private:
	const std::vector< ServerNode > &_ast;
	std::map< std::string, ServerSetter > _serverDispatch;
	std::map< std::string, LocationSetter > _locationDispatch;

public:
	ConfigBuilder(const std::vector< ServerNode > &ast);
	~ConfigBuilder();
	std::map< ListenKey, std::vector< ServerConfig > > buildAll();
	ServerConfig buildServer(const ServerNode &node);
	LocationConfig buildLocation(const LocationNode &node);

private:
	void initServerDispatch();
	void initLocationDispatch();

	// --- Server setters ---
	static void set_listen(ServerConfig &, const Directive &);
	static void set_root(ServerConfig &, const Directive &);
	static void set_server_name(ServerConfig &, const Directive &);
	static void set_index(ServerConfig &, const Directive &);
	static void set_client_max_body(ServerConfig &, const Directive &);
	static void set_keepalive(ServerConfig &, const Directive &);
	static void set_error_page(ServerConfig &, const Directive &);
	static void set_autoindex(ServerConfig &, const Directive &);
	static void set_allow(ServerConfig &, const Directive &);

	// --- Location setters ---
	static void loc_set_root(LocationConfig &, const Directive &);
	static void loc_set_index(LocationConfig &, const Directive &);
	static void loc_set_allow(LocationConfig &, const Directive &);
	static void loc_set_cgi(LocationConfig &, const Directive &);
	static void loc_set_upload_enable(LocationConfig &lc, const Directive &d);
	static void loc_set_upload_path(LocationConfig &, const Directive &);
	static void loc_set_redirect(LocationConfig &, const Directive &);
	static void loc_set_autoindex(LocationConfig &, const Directive &);
};

#endif

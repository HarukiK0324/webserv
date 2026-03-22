#include "../include/PortManager.hpp"
#include "../include/Utils.hpp"

#define LISTEN_BUF_SIZE 128
#define LOCAL_HOST "127.0.0.1"

// ipv 通信のverはとりあえずipv4で固定

PortManager::PortManager()
{
}
PortManager::~PortManager()
{
}

// struct addrinfo
// {
// 	int ai_flags;			   // AI_PASSIVE (サーバー用) などのオプション
// 	int ai_family;			   // AF_INET (IPv4) か AF_INET6 (IPv6) か
// 	int ai_socktype;		   // SOCK_STREAM (TCP) か SOCK_DGRAM (UDP) か
// 	int ai_protocol;		   // プロトコル (通常は0)
// 	socklen_t ai_addrlen;	   // ai_addr の大きさ
// 	struct sockaddr *ai_addr;  // 実際のIPアドレス情報へのポインタ
// 	char *ai_canonname;		   // ホストの正式名称
// 	struct addrinfo *ai_next;  // 次の構造体へのポインタ (リスト構造)
// };
PortManager::PortManager(const std::map< ListenKey, std::vector< ServerConfig > > &ServerConfigs)
{
	for (std::map< ListenKey, std::vector< ServerConfig > >::const_iterator it = ServerConfigs.begin(); it != ServerConfigs.end(); ++it)
	{
		const ListenKey &key = it->first;
		// --- getaddrinfo 用設定 ---
		struct addrinfo hints;
		struct addrinfo *res;
		std::memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;  // host == NULL のとき INADDR_ANY

		const char *host = (key.host == "*" || key.host == "0.0.0.0") ? NULL : key.host.c_str();
		const std::string &port_str = it->second[0].listen_port;
		int ret = getaddrinfo(host, port_str.c_str(), &hints, &res);
		if (ret != 0)
			throw std::runtime_error("getaddrinfo failed");
		int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd < 0)
		{
			freeaddrinfo(res);
			throw std::runtime_error("socket miss");
		}
		std::cout << "open listen " << key.host << ":" << key.port << std::endl;
		// display_fd(fd);
		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		int flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		if (bind(fd, res->ai_addr, res->ai_addrlen) < 0)
		{
			safty_close(fd);
			freeaddrinfo(res);
			throw std::runtime_error("bind miss\nport:" + port_str + "host:" + key.host);
		}
		freeaddrinfo(res);
		if (listen(fd, LISTEN_BUF_SIZE) < 0)
		{
			safty_close(fd);
			throw std::runtime_error("listen miss\nport:" + port_str + "host:" + key.host);
		}
		//
		MakePortfdAndConfs(fd, it->second);
	}  // --- fdと ServerConfig 群を紐づけ ---
}

void PortManager::MakePortfdAndConfs(int fd, const std::vector< ServerConfig > &sconfs)
{
	for (std::vector< ServerConfig >::const_iterator it = sconfs.begin(); it != sconfs.end(); ++it)
	{
		const ServerConfig &sconf = *it;
		for (std::vector< std::string >::const_iterator name_it = sconf.server_name.begin(); name_it != sconf.server_name.end(); ++name_it)
			_PortfdAndConfs[fd][*name_it] = sconf;
	}
}

std::map< int, std::map< std::string, ServerConfig > > &PortManager ::getPortfdAndConfs()
{
	return _PortfdAndConfs;
}
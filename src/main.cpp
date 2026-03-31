
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "ConfigBuilder.hpp"
#include "ConfigParser.hpp"
#include "PortManager.hpp"
#include "ServerLoop.hpp"
#include "Utils.hpp"
#include <signal.h>
#include <sys/wait.h>

static void handle_sigchld(int sig)
{
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

static void handle_sigint(int sig)
{
	(void)sig;
	server_run = 0;
}

void setup_signal()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, handle_sigchld);
	signal(SIGINT, handle_sigint);
}
//課題 errnoをみて制御しているのがよろしくない。　各ファイル関連のシステムコールのエラー処理について理解する(失敗::-1　成功::0)
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}
	std::srand(std::time(NULL));
	try
	{
		// 設定ファイルの解析 throw　→ main関数でキャッチ(return 1)
		ConfigParser parser(argv[argc - 1]);
		parser.lexer();
		parser.parser();
		ConfigBuilder builder(parser.getAst());
		std::map< ListenKey, std::vector< ServerConfig > > sconfs = builder.buildAll();
		setup_signal();
		PortManager pM(sconfs);
		// poll の処理準備をする。server_loopの中でイベントハンドラーを作成して行く感じ
		std::cout << "Starting server..." << std::endl;
		ServerLoop loop(pM.getPortfdAndConfs());
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
}

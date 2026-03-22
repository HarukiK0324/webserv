
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "../include/ConfigBuilder.hpp"
#include "../include/ConfigParser.hpp"
#include "../include/PortManager.hpp"
#include "../include/ServerLoop.hpp"
#include "../include/Utils.hpp"

static void handle_sig(int sig)
{
	write(g_sig_pipe[WRITE_FD], &sig, sizeof(sig));
}

void setup_signal()
{
	if (pipe(g_sig_pipe) < 0)
	{
		throw std::runtime_error("pipe error");
		return;
	}
	signal(SIGPIPE, handle_sig);
	signal(SIGCHLD, handle_sig);
	fcntl(g_sig_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(g_sig_pipe[1], F_SETFL, O_NONBLOCK);
	signal(SIGINT, handle_sig);
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

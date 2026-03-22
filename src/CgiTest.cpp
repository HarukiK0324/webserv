#include "../include/CgiParser.hpp"
#include "../include/Utils.hpp"

int main(int argc, char *argv[])
{
	CgiParser parser;

	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <cgi_response_file>" << std::endl;
		return 1;
	}
	int fd = open(argv[argc - 1], O_RDONLY);
	char buf[BUFSIZ];
	ssize_t n;							// readの戻り値はssize_tが適切です
	while ((n = read(fd, buf, 7)) > 0)	// 10バイトずつ読み込み
	{
		// parserへの渡し方はそのままでOK（nを渡しているため）
		parser.ReadResponse(buf, n);
	}
	parser.ReadResponse(NULL, 0);
	parser.printResult();
	safty_close(fd);
}
#include "../include/SignalHandler.hpp"
#include "../include/ServerLoop.hpp"

SignalHandler::SignalHandler(ServerLoop &loop) :
	FdHandler(g_sig_pipe[READ_FD], POLLIN, loop), _sig()
{
}

SignalHandler ::~SignalHandler()
{
	safty_close(g_sig_pipe[READ_FD]);
	safty_close(g_sig_pipe[WRITE_FD]);
}

void SignalHandler ::ReadEvent(Event ev)
{
	(void)ev;
	while (1)
	{
		int n = read(g_sig_pipe[READ_FD], &_sig, sizeof(_sig));
		if (n == -1)
			break;
		if (n == EOF)
			break;
		switch (_sig)
		{
			case SIGINT:
				_loop.StopServer();
				_sig = 0;
				break;
			case SIGCHLD:
				while (waitpid(-1, NULL, WNOHANG) > 0)
					;
				_sig = 0;
				break;
			case SIGPIPE:
				_sig = 0;
				break;
			default:
				break;
		}
	}
}
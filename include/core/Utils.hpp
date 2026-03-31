#ifndef Utils_HPP
#define Utils_HPP
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <sstream>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

#define WRITE_FD 1
#define READ_FD 0
#define BUFLEN 16384
void safty_close(int &fd);
std::string to_string(int n);
void print_location(const LocationConfig &lc);
void print_server(const std::vector< ServerConfig > &svec);
bool decodePercentEncoded(const std::string &input, std::string &output);
bool normalizeRequestPath(const std::string &requestTarget, std::string &normalizedPath);
bool resolvePathUnderRoot(const std::string &rootDir, const std::string &requestPath, std::string &resolvedPath);

#endif
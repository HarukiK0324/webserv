#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "Config.hpp"
#include "Token.hpp"

class Token;

class ConfigParser
{
private:
	std::stringstream _buf;
	std::string _alldate;
	std::vector< Token > _tokens;
	std::vector< Token >::iterator _token_it;
	std::vector< ServerNode > _ast;
	ServerNode server();
	LocationNode location();
	Directive directive();
	void error(const std::string &msg);
	bool consume(Token::TokenType ty);
	/* data */
public:
	ConfigParser(const std::string &filename);
	~ConfigParser();
	void lexer();
	void parser();
	const std::vector< ServerNode > &getAst() const;
	void printAST() const;
	void printTokens() const;
};
#endif

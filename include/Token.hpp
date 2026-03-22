#ifndef TOKEN_HPP
#define TOKEN_HPP
#include <string>
#include <iostream>
#include "Config.hpp"

class Token
{
public:
	enum TokenType
	{
		TOKEN_SERVER,
		TOKEN_LOCATION,
		TOKEN_BRACE_OPEN,
		TOKEN_BRACE_CLOSE,
		TOKEN_SEMICOLON,
		TOKEN_STR,
	};

private:
	TokenType _type;
	std::string _value;
	void dis_log();

public:
	Token(/* args */);
	Token(std::string &str, TokenType type);
	Token(char c, TokenType type);
	~Token();
	bool IsSameType(TokenType t) const;
	std::string TypeToStr();
	std::string &GetValue();
};

#endif
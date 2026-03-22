#include "../include/Token.hpp"

Token::~Token()
{
}
Token ::Token(char c, TokenType type) :
	_type(type), _value(1, c)
{
	// dis_log();
}

Token::Token(std::string &str, TokenType type) :
	_type(type), _value(str)
{
	if (_value == SERVER)
		_type = TOKEN_SERVER;
	if (_value == LOCATION)
		_type = TOKEN_LOCATION;
	// dis_log();
}

bool Token ::IsSameType(TokenType t) const
{
	return t == _type;
}

std::string &Token ::GetValue()
{
	return _value;
}
std::string Token::TypeToStr()
{
	switch (_type)
	{
		case TOKEN_SERVER:
			return "SERVER";

		case TOKEN_LOCATION:
			return "LOCATION";

		case TOKEN_BRACE_OPEN:
			return "BRACE_OPEN";

		case TOKEN_BRACE_CLOSE:
			return "BRACE_CLOSE";

		case TOKEN_SEMICOLON:
			return "SEMICOLON";

		case TOKEN_STR:
			return "STR";

		default:
			return "UNKNOWN";
	}
}

void Token::dis_log()
{
	std::cout << "TOKEN_TYPE = " << TypeToStr() << " value = " << _value << std::endl;
}
#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string &filename)
{
	if (filename.size() > 5 && filename.substr(filename.size() - 5) != ".conf")
		throw std::runtime_error("Config file must have a .conf extension");
	std::ifstream conf_file(filename.c_str());
	if (!conf_file.is_open())
		throw std::runtime_error("Please provide a valid .conf file");
	_buf << conf_file.rdbuf();
	_alldate = _buf.str();
}

ConfigParser::~ConfigParser()
{
}

const std::vector< ServerNode > &ConfigParser::getAst() const
{
	return _ast;
}

void ConfigParser::lexer()
{
	size_t n = _alldate.size();

	for (size_t i = 0; i < n;)
	{
		switch (_alldate[i])
		{
			case '{':
				_tokens.push_back(Token('{', Token::TOKEN_BRACE_OPEN));
				i++;
				break;
			case '}':
				_tokens.push_back(Token('}', Token::TOKEN_BRACE_CLOSE));
				i++;
				break;
			case ';':
				_tokens.push_back(Token(';', Token::TOKEN_SEMICOLON));
				i++;
				break;
			default:  //case STR
				while (i < n && isspace(_alldate[i]))
					i++;
				std::string value;
				while (i < n && !(isspace(_alldate[i]) || _alldate[i] == '}' || _alldate[i] == '{' || _alldate[i] == ';'))
					value += _alldate[i++];
				if (!value.empty())
				{
					_tokens.push_back(Token(value, Token::TOKEN_STR));
					continue;
				}
		}
	}
}

bool ConfigParser::consume(Token::TokenType ty)
{
	if (_token_it != _tokens.end() && _token_it->IsSameType(ty))
	{
		_token_it++;
		return true;
	}
	return false;
}

void ConfigParser::parser()
{
	_token_it = _tokens.begin();
	while (_token_it != _tokens.end())
	{
		ServerNode snode = server();
		_ast.push_back(snode);
	}
}

ServerNode ConfigParser::server()
{
	if (!consume(Token::TOKEN_SERVER))
		error("Expected 'server'");

	if (!consume(Token::TOKEN_BRACE_OPEN))
		error("Missing '{' after 'server'");

	ServerNode snode;

	while (!consume(Token::TOKEN_BRACE_CLOSE))
	{
		if (consume(Token::TOKEN_LOCATION))
		{
			LocationNode lnode = location();
			snode.locations[lnode.path] = lnode;
		}
		else
			snode.directives.push_back(directive());
	}

	return snode;
}

LocationNode ConfigParser::location()
{
	LocationNode lnode;

	if (!_token_it->IsSameType(Token::TOKEN_STR))
		error("Location block requires a path");

	lnode.path = _token_it->GetValue();
	_token_it++;

	if (!consume(Token::TOKEN_BRACE_OPEN))
		error("Location block must start with '{'");

	while (!consume(Token::TOKEN_BRACE_CLOSE))
	{
		if (_token_it == _tokens.end())
			error("Unexpected end of file in location block");
		lnode.directives.push_back(directive());
	}
	return lnode;
}

Directive ConfigParser::directive()
{
	Directive d;

	if (!_token_it->IsSameType(Token::TOKEN_STR))
		error("Directive key expected");

	d.key = _token_it->GetValue();
	_token_it++;
	if (!_token_it->IsSameType(Token::TOKEN_STR))
		error("Directive requires at least one value");
	while (_token_it->IsSameType(Token::TOKEN_STR))
	{
		d.values.push_back(_token_it->GetValue());
		_token_it++;
	}

	if (!consume(Token::TOKEN_SEMICOLON))
		error("Missing ';' at end of directive");

	return d;
}
void ConfigParser::error(const std::string &msg)
{
	std::stringstream ss;

	if (_token_it != _tokens.end())
	{
		ss << "Parse error near token: \""
		   << _token_it->GetValue()
		   << "\"\n";
	}
	else
	{
		ss << "Parse error at end of file\n";
	}
	ss << msg;
	throw std::runtime_error(ss.str());
}

void ConfigParser::printTokens() const
{
	std::cout << "\n===== TOKEN DUMP =====\n";
	for (size_t i = 0; i < _tokens.size(); i++)
	{
		Token t = _tokens[i];
		std::cout << "Token #" << i << ": Type=" << t.TypeToStr()
				  << ", Value=\"" << t.GetValue() << "\"\n";
	}
	std::cout << "======================\n";
}

void ConfigParser::printAST() const
{
	std::cout << "\n========== AST DUMP ==========\n";

	for (size_t si = 0; si < _ast.size(); si++)
	{
		const ServerNode &server = _ast[si];

		// ---- Server Node バーナー ----
		std::cout << "\n===== SERVER NODE #" << si << " =====\n";

		// ---- Server directives ----
		std::cout << "--- SERVER DIRECTIVES ---\n";
		if (server.directives.empty())
			std::cout << "  (no directives)\n";

		for (size_t di = 0; di < server.directives.size(); di++)
		{
			const Directive &d = server.directives[di];
			std::cout << "  " << d.key;

			for (size_t vi = 0; vi < d.values.size(); vi++)
				std::cout << " " << d.values[vi];

			std::cout << ";\n";
		}

		// ---- Location blocks ----
		for (std::map< std::string, LocationNode >::const_iterator it = server.locations.begin();
			it != server.locations.end();
			++it)
		{
			const std::string &path = it->first;
			const LocationNode &loc = it->second;

			// Location のバーナー
			std::cout << "\n--- LOCATION path = " << path << " ---\n";

			// Location 内のディレクティブ
			std::cout << "--- LOCATION DIRECTIVES ---\n";
			if (loc.directives.empty())
				std::cout << "    (no directives)\n";

			for (size_t di = 0; di < loc.directives.size(); di++)
			{
				const Directive &d = loc.directives[di];
				std::cout << "    " << d.key;

				for (size_t vi = 0; vi < d.values.size(); vi++)
					std::cout << " " << d.values[vi];

				std::cout << ";\n";
			}
		}
	}

	std::cout << "\n================================\n";
}

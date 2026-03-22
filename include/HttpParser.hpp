#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "StartLineReader.hpp"
#include "HeaderFieldReader.hpp"
#include "BodyReader.hpp"
#include "ChunkBodyReader.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "MimeTypes.hpp"
#include "HttpErrorCode.hpp"

#define CRLF "\r\n"
#define LF "\n"
#define SP " "
#define COLON ":"

/*
HTTP-message   = start-line
                 *( header-field CRLF )
                 CRLF
                 [ message-body ]
*/

class HttpParser
{
public:
	HttpParser(ServerConfig &config);
	~HttpParser();
	HttpParserState getState() const;
	HttpResponse getResponse(ServerConfig &config);
	HttpRequest getRequest() const
	{
		return _request;
	}
	void sendData(const std::string &buf, size_t len);
	HttpParserState _state;

private:
	void setError(ErrorCode code, const std::string &message);

	ErrorCode _error;
	std::string _error_message;
	//start-line = request-line / status-line
	//request-line = method SP request-target SP HTTP-version CRLF
	//absolute-path [ "?" query ]
	//header-field   = field-name ":" OWS field-value OWS
	HttpRequest _request;
	StartLineReader _start_line_reader;
	HeaderFieldReader _header_field_reader;
	BodyReader *_body_reader;
	std::string _raw_request;
	ServerConfig &_config;
};

std::ostream &operator<<(std::ostream &os, const HttpParserState &state);

#endif

//文法
// HTTP - message = start - line CRLF  * (field - line CRLF) CRLF[message - body]

// REQUEST_METHOD	GET / POST / HEAD など
// QUERY_STRING	? 以降のクエリ（URLエンコードされた文字列）
// CONTENT_LENGTH	本文のバイト数（POST のとき特に重要）
// CONTENT_TYPE	本文の種類（フォーム, JSON, ファイルなど）
// SCRIPT_NAME	スクリプト自身のパス（/cgi-bin/test.cgi など）
// PATH_INFO	スクリプトに続く追加パス（/test.cgi/123/abc の /123/abc 部分）
// SERVER_NAME	サーバーのホスト名（例：example.com）
// SERVER_PORT	受けたポート番号（80, 8080, 8000 など）
// SERVER_PROTOCOL	HTTP/1.1 など
// REMOTE_ADDR	クライアントの IP アドレ
// post
// サーバーは CONTENT_LENGTH バイトぶんを必ず用意しなければならない(MUST)
// スクリプトは CONTENT_LENGTH バイト以上は読んではいけない(MUST NOT)
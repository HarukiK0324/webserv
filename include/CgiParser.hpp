#ifndef CGI_PARSER
#define CGI_PARSER

#define CRLF "\r\n"
#define LF "\n"
#define SP " "
#define COLON ":"

#include <iostream>
#include <vector>
#include <map>
#include <string>

class CgiParser
{
public:
	enum State
	{
		HEADER_FIELDS,
		BODY,
		DONE,
		ERROR
	};
	enum ResponseType
	{
		DOCUMENT_RESPONSE,
		LOCAL_REDIR_RESPONSE,
		CLIENT_REDIR_RESPONSE,
		CLIENT_REDIR_DOC_RESPONSE,
		UNKNOWN_RESPONSE
	};
	enum StatusCode	 //status-code
	{
		// 1xx: informational
		// 2xx: successful
		// 3xx: redirection
		// 4xx: client error
		// 5xx: server error
		OK = 200,
		FOUND = 302,
		BAD_REQUEST = 400,
		FORBIDDEN = 403,
		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED = 501,
		BAD_GATEWAY = 502,
		SERVICE_UNAVAILABLE = 503
	};

private:
	State _state;
	ResponseType _responseType;
	StatusCode _statusCode;
	size_t _offset;
	std::vector< char > _responseBuffer;
	std::vector< char > _body;
	std::map< std::string, std::string > _headers;
	std::string _header_line;

	void parser();
	void handleClientRedirect();
	std::string GetHeaderLine();
	void ParseHeaderKV(const std::string &line, std::string &key, std::string &value);
	void ReadHeaders();

	void setState(const State &state);
	void setResponseType(const ResponseType &responsetype)
	{
		_responseType = responsetype;
	};
	void setHeader(const std::string &key, const std::string &value);
	/* data */
public:
	CgiParser();
	~CgiParser();
	//処理関数
	std::vector< char > getBody() const;
	void ReadResponse(char *buf, size_t len);
	void printResult();
	void setStatusCode(const StatusCode &statusCode)
	{
		_statusCode = statusCode;
	};
	//getters
	const ResponseType &getResuponseType() const
	{
		return _responseType;
	};
	const StatusCode &getStatusCode() const
	{
		return _statusCode;
	};
	const State &getState() const
	{
		return _state;
	};
	const std::map< std::string, std::string > &getHeaders() const
	{
		return _headers;
	};
};

std::ostream &operator<<(std::ostream &os, const CgiParser::State &state);
std::ostream &operator<<(std::ostream &os, const CgiParser::StatusCode &statusCode);
std::ostream &operator<<(std::ostream &os, const CgiParser::ResponseType &responsetype);

// std::vector< char > _body;	//未使用
// std::string _result;  //未使用

//	ABNF //status　コードの決定必要
// document - response =
// 	 Content - Type
// 	 [Status] あったらそれに　ないならデフォル　２００
// 	 * other - field
// 	 NL
// 	 response - body

// local-redir-response =
// 	 local-Location
// 	 NL

// client-redir-response =
// 	 client-Location
// 	 *extension-field　？
// 	  NL

// client-redirdoc-response =
//      client-Location
//      Status　　　３０２固定
//      Content-Type
//      *other-field
//      NL
//      response-body

//重複出現、禁止   CGI-field = Content-Type | Location | Status

//   header-field    = CGI-field | other-field
//   CGI-field       = Content-Type | Location | Status
//   other-field     = protocol-field | extension-field
//   protocol-field  = generic-field	クッキーなど
//   extension-field = generic-field	　
//   generic-field   = field-name ":" [ field-value ] NL
//   field-name      = token0
//   field-value     = *( field-content | LWSP )
//   field-content   = *( token | separator | quoted-string )

// Location = local - Location | client - Location
// client - Location = "Location:" fragment - URI NL
// local - Location = "Location:" local - pathquery NL
// client系のときの　URI
// fragment - URI = absoluteURI["#" fragment]
// absoluteURI = scheme ":" hier - part [ "?" query - string ]
// scheme = alpha *( alpha | digit | "+" | "-" | "." )
// hier - part = "//" authority [ path - absolute ] | path - absolute

//　local系のときの path
// local - pathquery = abs - path["?" query - string]
// abs - path = "/" path - segments　絶対パス　関数作成時に重要な文法
// path - segments = segment * ("/" segment)
// segment = *pchar
// pchar = unreserved | escaped | extra extra = ":" | "@" | "&" | "=" | "+" | "$" | "," 予約語以外何でもOK

#endif
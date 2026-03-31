#ifndef LOCATION_CONFIG_HPP
#define LOCATION_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstdlib>
struct LocationConfig;

//複数回の定義を許すか、許さないかで、flagの有無が変わる
// ----- raw values -----

struct LocationConfig
{
	std::string path;  // HTTPリクエストでのパス
	//directive
	std::string root;						   //method sever内部のパス
	std::vector< std::string > index;		   //method
	std::vector< std::string > allow_methods;  //HttpPlotocol(State Method)
	std::string cgi_type, CgiExecuter;		   //cgihandler
	bool upload_enable;						   //PostMethod
	std::string upload_path;				   //PostMethod
	bool autoindex;
	bool redirect;	//GetMetod （リクエストがディレクトリ）　ディレクトリリスティング
	int redirect_code;
	std::string redirect_target;
	//GetMethod　リダイレクト設定　codeとtarget
	//StatusCodeは300
	//
	// ====== flags ====== ServerConfigから継承するかどうかの判定
	bool root_set;
	bool index_set;
	bool autoindex_set;
	bool allow_methods_set;

	LocationConfig() :
		upload_enable(false),
		autoindex(false),
		redirect(false),
		redirect_code(0),
		root_set(false),
		index_set(false),
		autoindex_set(false),
		allow_methods_set(false) {};
};

#endif

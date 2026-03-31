#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <map>
#include <ctime>

class Session
{
public:
    Session() : _session_id(""), _expires_at(0), _data() {}
	Session(const std::string &id, std::time_t expires, std::map<std::string, std::string> data) : _session_id(id), _expires_at(expires), _data(data) {}
    ~Session() {}

    std::string getSessionId() const { return _session_id; }
    std::time_t getExpiresAt() const { return _expires_at; }
    void setExpiresAt(std::time_t expires) { _expires_at = expires; }
    void setData(const std::string &key, const std::string &value) { _data[key] = value; }
    void removeData(const std::string &key) { _data.erase(key); }
    std::string getData(const std::string &key) const;
    
    static std::string getNewSessionId();
private:
	std::string _session_id;
	std::time_t _expires_at;
	std::map<std::string, std::string> _data;
};


#endif
#include "Session.hpp"
#include <cstdio>
#include <cstdlib>

std::string Session::getData(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it = _data.find(key);
    if (it != _data.end()) {
        return it->second;
    }
    return "";
}

std::string Session::getNewSessionId() {
    static const char hex_chars[] = "0123456789abcdef";
    unsigned char bytes[16];
    bool got_random = false;
    std::FILE *urandom = std::fopen("/dev/urandom", "rb");
    if (urandom) {
        got_random = (std::fread(bytes, 1, sizeof(bytes), urandom) == sizeof(bytes));
        std::fclose(urandom);
    }
    if (!got_random) {
        for (int i = 0; i < 16; ++i)
            bytes[i] = static_cast<unsigned char>(std::rand() % 256);
    }
    std::string new_session_id = "";
    new_session_id.reserve(32);
    for (int i = 0; i < 16; ++i) {
        new_session_id += hex_chars[(bytes[i] >> 4) & 0x0f];
        new_session_id += hex_chars[bytes[i] & 0x0f];
    }
    return new_session_id;
}

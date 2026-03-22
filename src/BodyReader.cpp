#include "../include/BodyReader.hpp"

bool BodyReader::parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config)
{
    if(request.getHeader("Content-Length") == "" || _to_read <= _read_bytes)
    {
        return true;
    }
    size_t len = std::min(_to_read - _read_bytes, raw_request.length());
    if(len + _read_bytes > config.client_max_body_size)
    {
        setError(PAYLOAD_TOO_LARGE, "Body too large");
        return false;
    }
    request.addBody(raw_request.substr(0, len));
    _read_bytes += len;
    raw_request.erase(0, len);
    if(_to_read <= _read_bytes)
        return true;
    return false;
}

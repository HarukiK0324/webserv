#include "ChunkBodyReader.hpp"

bool ChunkBodyReader::parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config)
{
    while(true)
    {
        if(_chunk_state == SIZE)
        {
            size_t pos = raw_request.find(CRLF);
            if(pos == std::string::npos)
                return false;
            _chunk_size = strtol(raw_request.substr(0, pos).c_str(), NULL, 16);
            raw_request.erase(0, pos + 2);
            if(raw_request[pos-1] == '-')
            {
                setError(BAD_REQUEST, "Invalid chunk size");
                return false;
            }
            else if (_chunk_size == 0) {
                return true;
            }
            _chunk_state = DATA;
        }
        if(_chunk_state == DATA)
        {
            if(raw_request.length() < _chunk_size + 2)
                return false;
            if(raw_request.find(CRLF, _chunk_size) != _chunk_size)
            {
                setError(BAD_REQUEST, "Invalid chunked encoding");
                return false;
            }
            request.addBody(raw_request.substr(0, _chunk_size));
            if(request.getBody().length() > config.client_max_body_size)
            {
                setError(PAYLOAD_TOO_LARGE, "Body too large");
                return false;
            }
            raw_request.erase(0, _chunk_size + 2); // +2 for CRLF
            _chunk_state = SIZE;
        }
    }
}
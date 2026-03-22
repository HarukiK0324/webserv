#ifndef CHUNKBODYREADER_HPP
#define CHUNKBODYREADER_HPP

#include <string>
#include <cstdlib>
#include "BodyReader.hpp"

class ChunkBodyReader : public BodyReader
{
    public:
        ChunkBodyReader() : BodyReader(), _chunk_state(SIZE), _chunk_size(0) {}
        ~ChunkBodyReader() {}

        bool parse(std::string& raw_request, HttpRequest& request, const ServerConfig& config);
    private:
        enum ChunkState {
            SIZE,
            DATA,
            TRAILER
        };
        ChunkState  _chunk_state;
        size_t      _chunk_size;
};

#endif
#ifndef HTTPSTATE_HPP
#define HTTPSTATE_HPP

enum HttpParserState
{
    START_LINE,
    HEADER_FIELDS,
    BODY,
    DONE,
    ERROR
};

#endif
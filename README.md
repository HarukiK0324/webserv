_This project has been created as part of the 42 curriculum by **tohbu** and **hkasamat**._

## Description

**webserv** is an HTTP/1.1 web server written in C++98, developed as a 42 school project.  
It is inspired by NGINX and implements the core features required to serve static and dynamic web content.

Key characteristics:

- Non-blocking I/O using `poll()`
- Full HTTP/1.1 request parsing (start line, headers, body, chunked transfer encoding)
- Static file serving (GET, POST, DELETE)
- CGI support for dynamic content (e.g. Python scripts)
- Virtual host and multi-port support via a configuration file
- Autoindex (directory listing), custom error pages, and file uploads

## Instructions

### Requirements

- C++ compiler supporting `-std=c++98` (e.g. `c++`)
- GNU Make
- Python 3 (for CGI scripts)
- Linux / macOS

### Build

```bash
# Build the main server
make

# Build the configuration parser tester
make conf_test

# Clean object files
make clean

# Clean everything (objects + binaries)
make fclean

# Rebuild from scratch
make re
```

### Usage

```bash
./webserv <config_file>
```

Example:

```bash
./webserv conf/sample.conf
```

### Configuration

The server is configured through a `.conf` file with NGINX-like syntax.

#### Basic example

```nginx
server {
    listen 127.0.0.1:8080;
    server_name example.com www.example.com;
    root /var/www/html;
    index index.html;
    autoindex off;
    client_max_body_size 1000000;
    error_page 404 /error_pages;

    location /cgi-bin {
        cgi_pass .py /usr/bin/python3;
        allow_methods GET POST;
    }

    location /upload {
        upload_path /var/www/files;
    }

    location /redirect {
        return 301 http://www.example.com/;
    }
}
```

## Resources

- [RFC 7230 – HTTP/1.1: Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 – HTTP/1.1: Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [RFC 3875 – CGI/1.1 Specification](https://datatracker.ietf.org/doc/html/rfc3875)
- [NGINX Configuration Guide](https://nginx.org/en/docs/beginners_guide.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

# !/usr/bin/env python3
import os

def dump_cgi_env():
    cgi_env_keys = [
        "PATH_INFO",
        "PATH_TRANSLATED",
        "QUERY_STRING",
        "REMOTE_ADDR",
        "REMOTE_HOST",
        "REMOTE_IDENT",
        "REMOTE_USER",
        "REQUEST_METHOD",
        "SCRIPT_NAME",
        "SERVER_NAME",
        "SERVER_PORT",
        "SERVER_PROTOCOL",
        "SERVER_SOFTWARE",
        "CONTENT_TYPE",
        "CONTENT_LENGTH",
    ]

    print("Content-Type: text/plain")
    print()

    print("=== CGI Environment Variables ===")
    for key in cgi_env_keys:
        value = os.environ.get(key)
        if value is None:
            print(f"{key}: (unset)")
        else:
            print(f"{key}: {value}")
    print("=================================")
    print("body:")
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length > 0:
            body = os.sys.stdin.read(content_length)
            print(body)
        else:
            print("(no body)")
    except (ValueError, OSError):
        print("(error reading body)")
if __name__ == "__main__":
    dump_cgi_env()
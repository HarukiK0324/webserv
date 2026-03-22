#!/usr/bin/env python3
import os
import sys

http_cookie = os.environ.get("HTTP_COOKIE", "")
session_id = ""
for pair in http_cookie.split(";"):
    pair = pair.strip()
    if pair.startswith("session_id="):
        session_id = pair[len("session_id="):]
        break

print("Content-Type: text/plain")
print("Status: 200 OK")
print()
print("HTTP_COOKIE=" + http_cookie)
print("session_id=" + session_id)

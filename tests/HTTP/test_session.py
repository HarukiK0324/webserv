"""
Integration tests for cookie and session management.

These tests verify that the webserv correctly:
- Creates a new session on the first request (Set-Cookie header returned)
- Accepts and reuses a valid existing session (no new Set-Cookie)
- Forwards the cookie to CGI scripts via HTTP_COOKIE env variable
"""

import os
import subprocess
import tempfile
import time
import re
from pathlib import Path

import pytest

from http_utils import build_http_request, send_request, get_status_code

ROOT = Path(__file__).resolve().parents[2]
WEBSERV = ROOT / "webserv"
CONFIG = ROOT / "conf/test.conf"


def _write_conf(path: str) -> None:
    """Write the server configuration with paths resolved relative to the project root."""
    html_root = ROOT / "var" / "www" / "html"
    cgi_root = ROOT / "var" / "www"
    conf = (
        "server {{\n"
        "    listen 127.0.0.1:8080;\n"
        "    server_name localhost;\n"
        "    root {html_root};\n"
        "    index index.html;\n"
        "    client_max_body_size 1000000;\n"
        "\n"
        "    location / {{\n"
        "        allow_methods GET POST;\n"
        "    }}\n"
        "\n"
        "    location /cgi-bin {{\n"
        "        root {cgi_root};\n"
        "        cgi_pass .py /usr/bin/python3;\n"
        "        allow_methods GET POST;\n"
        "    }}\n"
        "}}\n"
    ).format(html_root=html_root, cgi_root=cgi_root)
    with open(path, "w") as f:
        f.write(conf)


def parse_set_cookie(response: bytes, name: str) -> str:
    """
    Extract the value of a named cookie from a Set-Cookie response header.

    Returns the cookie value string, or "" if not found.
    """
    for line in response.split(b"\r\n"):
        if line.lower().startswith(b"set-cookie:"):
            header_val = line[len(b"set-cookie:"):].strip().decode("ascii", errors="replace")
            for part in header_val.split(";"):
                part = part.strip()
                if part.lower().startswith(name.lower() + "="):
                    return part[len(name) + 1:]
    return ""


def parse_response_body(response: bytes) -> str:
    """Return the body of an HTTP response (after the blank line)."""
    separator = b"\r\n\r\n"
    pos = response.find(separator)
    if pos == -1:
        return ""
    return response[pos + len(separator):].decode("utf-8", errors="replace")

def parse_response_headers(response: bytes) -> dict:
    """Return a dictionary of HTTP response headers."""
    headers = {}
    header_part = response.split(b"\r\n\r\n", 1)[0]
    for line in header_part.split(b"\r\n")[1:]:  # Skip status line
        if b":" in line:
            name, value = line.split(b":", 1)
            headers[name.strip().lower()] = value.strip()
    return headers

@pytest.fixture(scope="module")
def server():
    """Start the webserv process and stop it after the test module finishes."""
    # with tempfile.NamedTemporaryFile(mode="w", suffix=".conf", delete=False) as tmp:
    #     conf_path = tmp.name
    try:
        # _write_conf(conf_path)
        proc = subprocess.Popen(
            [str(WEBSERV), str(CONFIG)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        time.sleep(0.5)
        yield
        proc.terminate()
        proc.wait()
    finally:
        # os.unlink(conf_path)
        pass


class TestNewSessionCreation:
    """A request without a cookie should receive a Set-Cookie: session_id header."""

    def test_first_request_returns_set_cookie(self, server):
        req = build_http_request("GET", "/")
        res = send_request(req)
        session_id = parse_set_cookie(res, "session_id")
        assert session_id != "", "Expected Set-Cookie: session_id to be set on first request"

    def test_session_id_is_32_hex_characters(self, server):
        req = build_http_request("GET", "/")
        res = send_request(req)
        session_id = parse_set_cookie(res, "session_id")
        assert re.fullmatch(r"[0-9a-f]{32}", session_id), (
            f"session_id '{session_id}' is not 32 lowercase hex characters"
        )

    def test_session_cookie_has_httponly(self, server):
        req = build_http_request("GET", "/")
        res = send_request(req)
        for line in res.split(b"\r\n"):
            if line.lower().startswith(b"set-cookie:"):
                header_val = line.decode("ascii", errors="replace").lower()
                assert "httponly" in header_val, (
                    "Set-Cookie header should contain HttpOnly attribute"
                )
                break

    def test_session_cookie_has_path(self, server):
        req = build_http_request("GET", "/")
        res = send_request(req)
        for line in res.split(b"\r\n"):
            if line.lower().startswith(b"set-cookie:"):
                header_val = line.decode("ascii", errors="replace").lower()
                assert "path=/" in header_val, (
                    "Set-Cookie header should contain Path=/ attribute"
                )
                break


class TestSessionReuse:
    """A request that sends a valid session cookie should reuse that session."""

    def test_valid_session_not_replaced(self, server):
        # First request: get a session_id
        req1 = build_http_request("GET", "/")
        res1 = send_request(req1)
        session_id = parse_set_cookie(res1, "session_id")
        assert session_id != "", "Setup: expected to receive a session_id"

        # Second request: send the session cookie back
        req2 = build_http_request("GET", "/", header_fields={"Cookie": "session_id=" + session_id})
        res2 = send_request(req2)

        # Should get 200 OK
        assert get_status_code(res2) == 200

        # A new Set-Cookie should NOT be issued for a valid existing session
        new_session_id = parse_set_cookie(res2, "session_id")
        assert new_session_id == "", (
            "Server should not issue a new Set-Cookie when the existing session is valid"
        )

    def test_unknown_session_id_triggers_new_session(self, server):
        fake_id = "deadbeefdeadbeefdeadbeefdeadbeef"
        req = build_http_request(
            "GET", "/", header_fields={"Cookie": "session_id=" + fake_id}
        )
        res = send_request(req)

        # The unknown session should be replaced with a fresh one
        new_session_id = parse_set_cookie(res, "session_id")
        assert new_session_id != "", (
            "Server should issue a new Set-Cookie when the submitted session_id is unknown"
        )
        assert new_session_id != fake_id, (
            "New session_id should differ from the unknown/fake one submitted"
        )
        req = build_http_request(
            "GET", "/", header_fields={"Cookie": "session_id=" + new_session_id}
        )
        res = send_request(req)
        next_new_session_id = parse_set_cookie(res, "session_id")
        assert next_new_session_id == "", (
            "Server should not issue a new Set-Cookie when the existing session is valid"
        )


    def test_two_independent_requests_get_different_session_ids(self, server):
        req = build_http_request("GET", "/")
        res1 = send_request(req)
        res2 = send_request(req)
        id1 = parse_set_cookie(res1, "session_id")
        id2 = parse_set_cookie(res2, "session_id")
        assert id1 != "", "First request should receive a session_id"
        assert id2 != "", "Second request should receive a session_id"
        assert id1 != id2, "Each new session should get a unique session_id"


class TestCgiCookieForwarding:
    """The HTTP_COOKIE environment variable should be forwarded to CGI scripts."""

    def test_cgi_receives_http_cookie(self, server):
        req1 = build_http_request("GET", "/cgi-bin/session_demo.py")
        res1 = send_request(req1)
        session_id = parse_set_cookie(res1, "session_id")
        assert session_id != "", "Setup: expected to receive a session_id"

        req2 = build_http_request(
            "GET",
            "/cgi-bin/session_demo.py",
            header_fields={"Cookie": "session_id=" + session_id},
        )
        res2 = send_request(req2)
        assert get_status_code(res2) == 200
        body = parse_response_body(res2)
        assert "HTTP_COOKIE=session_id=" + session_id in body, (
            "CGI script should receive HTTP_COOKIE containing the session cookie"
        )

    def test_cgi_extracts_session_id_from_cookie(self, server):
        req1 = build_http_request("GET", "/cgi-bin/session_demo.py")
        res1 = send_request(req1)
        session_id = parse_set_cookie(res1, "session_id")
        assert session_id != "", "Setup: expected to receive a session_id"

        req2 = build_http_request(
            "GET",
            "/cgi-bin/session_demo.py",
            header_fields={"Cookie": "session_id=" + session_id},
        )
        res2 = send_request(req2)
        body = parse_response_body(res2)
        assert "session_id=" + session_id in body, (
            "CGI script should be able to extract the session_id from HTTP_COOKIE"
        )

    def test_cgi_no_cookie_http_cookie_empty(self, server):
        req = build_http_request("GET", "/cgi-bin/session_demo.py")
        res = send_request(req)
        assert get_status_code(res) == 200
        body = parse_response_body(res)
        # HTTP_COOKIE should be empty when no cookie is sent
        assert "HTTP_COOKIE=\n" in body, (
            "HTTP_COOKIE should be empty when no Cookie header is sent"
        )
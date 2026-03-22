import subprocess
import time
from pathlib import Path
from http_utils import build_http_request, send_request, get_status_code, status_line  , parse_response
import pytest


HTTP_PARSER_LIMIT = 100001
ROOT = Path(__file__).resolve().parents[2]  # プロジェクトルート
WEBSERV = ROOT / "webserv"
CONFIG = ROOT / "conf/test.conf"
STDCERR = ROOT / "logs/test_stderr.log"
BIGSTR = "A" * HTTP_PARSER_LIMIT
LONG_URL = "A" * 7200 # 大きなURL用の文字列

@pytest.fixture(scope="module")
def server():
    STDCERR.parent.mkdir(parents=True, exist_ok=True)
    proc = subprocess.Popen(
        [str(WEBSERV), str(CONFIG)],
        stdout=subprocess.DEVNULL,
        stderr=STDCERR.open("w"),
    )
    time.sleep(0.3)  # 起動待ち
    yield
    proc.terminate()

#parse error
@pytest.mark.parametrize(
    "method",
    ["GTE", "PUT", "123"],
)
def test_invalid_method(server, method):
    req = build_http_request(method, "/")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 501
    assert f"<title>{status_line(501)}</title>" in body
    
def test_invalid_http_version(server):
    req = build_http_request("GET", "/", http_version="HTTP/2.0")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 505
    assert f"<title>{status_line(505)}</title>" in body

def test_URL_too_long(server):
    req = build_http_request("GET", "/" + LONG_URL)
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 414
    assert f"<title>{status_line(414)}</title>" in body

def  test_header_too_long(server):
    headers = {"X-Long-Header": BIGSTR}
    req = build_http_request("GET", "/", header_fields=headers)
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 431
    assert f"<title>{status_line(431)}</title>" in body

def test_body_too_long(server):
    req = build_http_request("POST", "/upload", body=BIGSTR.encode())
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 413
    assert f"<title>{status_line(413)}</title>" in body


def test_bad_header(server):
    # ヘッダーフィールドにコロンがない場合
    req = build_http_request("GET", "/", bad_header=True)
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 400
    assert f"<title>{status_line(400)}</title>" in body



#parse ok

def test_redirect(server):
    req = build_http_request("GET", "/redirect")
    res = send_request(req)
    assert get_status_code(res) == 301


def test_GET_ok(server):
    req = build_http_request("GET", "/")
    res = send_request(req)
    assert get_status_code(res) == 200
    
## ４００番台エラー

def test_GET_forbidden(server):
    req = build_http_request("GET", "/files/forbidden.text")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 403
    assert f"<title>{status_line(403)}</title>" in body

def test_GET_not_found(server):
    req = build_http_request("GET", "/files/not_found.text")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 404
    assert f"<title>{status_line(404)}</title>" in body

def test_GET_method_not_allowed(server):
    req = build_http_request("GET", "/upload")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 405
    assert f"<title>{status_line(405)}</title>" in body


#post_method

def test_POST_create(server):
    body = "name=John&age=30"
    req = build_http_request("POST", "/files/newfile.text", body=body.encode())
    res = send_request(req)
    assert get_status_code(res) == 201

def test_POST_forbidden_directory(server):
    body = "name=John&age=30"
    req = build_http_request("POST", "/FilesPermissionDined/newfile.text", body=body.encode())
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 403
    assert f"<title>{status_line(403)}</title>" in body

def test_POST_method_not_allowed(server):
    body = "name=John&age=30"
    req = build_http_request("POST", "/", body=body.encode())
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 405
    assert f"<title>{status_line(405)}</title>" in body

#delete_method
def test_DELETE_no_content(server):
    req = build_http_request("DELETE", "/files/newfile.text")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 204


def test_DELETE_forbidden_file(server):
    req = build_http_request("DELETE", "/files/forbidden.text")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 403
    assert f"<title>{status_line(403)}</title>" in body



def test_DELETE_not_found(server):
    req = build_http_request("DELETE", "/files/not_found.text")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 404
    assert f"<title>{status_line(404)}</title>" in body


def test_DELETE_method_not_allowed(server):
    req = build_http_request("DELETE", "/")
    res = send_request(req)
    status_code, body = parse_response(res)
    assert status_code == 405
    assert f"<title>{status_line(405)}</title>" in body







import socket
from typing import Optional, Dict


CRLF ="\r\n"

def build_http_request(
	method: str,
	target: str,
	header_fields: Optional[Dict[str, str]] = None,
	body: bytes = b"",
	http_version: str = "HTTP/1.1",
	bad_header: bool = False
) -> bytes:
	"""
	Builds a raw HTTP request.

	Args:
		method (str): The HTTP method (e.g., 'GET', 'POST').
		target (str): The target for the request.
		header_fields (Optional[Dict[str, str]]): Additional header fields.
		body (bytes): The body of the request.
		http_version (str): The HTTP version (default is 'HTTP/1.1').

	Returns:
		bytes: The raw HTTP request as bytes.
	"""
	if header_fields is None:
		header_fields = {}
	req = f"{method} {target} {http_version}{CRLF}"

	if body and "Content-Length" not in header_fields:
		header_fields["Content-Length"] = str(len(body))

	if http_version == "HTTP/1.1" and "Host" not in header_fields:
		header_fields["Host"] = "localhost"

	if bad_header:
		header_fields["X-Bad-Header"] = "bad_value_without_colon"
	else:
		for key, value in header_fields.items():
			if bad_header:
				req += f"{key} {value}{CRLF}"  # コロンをスペースにして不正なヘッダーを作成
			else:
				req += f"{key}: {value}{CRLF}"


	req += CRLF
	return req.encode("ascii") + body


def send_request(
		request: bytes,
		host: str = "127.0.0.1",
		port: int = 8083,
		timeout: float = 5.0,
) ->bytes:
	"""
	Sends a raw HTTP request to the specified host and port.

	Args:
		request (bytes): The raw HTTP request to send.
		host (str): The target host (default is '127.0.0.1').
		port (int): The target port (default is 8083).
		timeout (float): The socket timeout in seconds (default is 5.0).

	Returns:
		bytes: The raw HTTP response received.
	"""

	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.settimeout(timeout)
	sock.connect((host, port))
	sock.sendall(request)
	"""Send the HTTP request and receive the response."""
	response = b""

	while True:
		try:
			data = sock.recv(4096)
		except ConnectionResetError:
			# Some server error paths close with RST; keep any bytes already received.
			break
		if not data:
			break
		response += data
	sock.close()
	return response

def get_status_code(response: bytes) -> int:
	"""
	Extracts the HTTP status code from a raw HTTP response.

	Args:
		response (bytes): The raw HTTP response.
	Returns:
		int: The HTTP status code.
	"""

	line = response.split(b"\r\n", 1)[0]
	parts = line.split(b" ")
	if len(parts) < 2:
		raise ValueError("Invalid HTTP response")
	return int(parts[1])


def get_default_error_page(response: bytes) -> bytes:
	"""
	Returns a default HTML error page for the given HTTP status code.

	Args:
		status_code (int): The HTTP status code for which to generate the error page.

	Returns:
		bytes: The HTML content of the default error page as bytes.
	"""
	response_str = response.decode("utf-8", errors="ignore")
	lines = response_str.split("\r\n")
	if not lines:
		return b""
	status_line = lines[0]
	parts = status_line.split(" ")
	if len(parts) < 2:
		return b""
	status_code = int(parts[1])		

def parse_response(response: bytes):
    """HTTPレスポンスを (status_code, body_str) に分解"""
    response_str = response.decode("utf-8", errors="ignore")

    header, _, body = response_str.partition("\r\n\r\n")

    # status line
    status_line = header.split("\r\n")[0]
    parts = status_line.split(" ")
    status_code = int(parts[1])

    return status_code, body

def status_line(code: int) -> str:
    return f"{code} {get_status_message(code)}"

def get_status_message(code: int) -> str:
    status_map = {
        200: "OK",
        201: "Created",
        204: "No Content",
        301: "Moved Permanently",
        302: "Found",
        400: "Bad Request",
        403: "Forbidden",
        404: "Not Found",
        405: "Method Not Allowed",
        408: "Request Timeout",
        413: "Payload Too Large",
        414: "URI Too Long",
        431: "Header Fields Too Large",
        500: "Internal Server Error",
        501: "Not Implemented",
        505: "HTTP Version Not Supported",
    }
    return status_map.get(code, "Unknown Status")

# !/usr/bin/env python3
import os

def search_boundary(content_type):
	if content_type is None:
		return None
	parts = content_type.split(";")
	for part in parts:
		part = part.strip()
		if part.startswith("boundary="):
			return part[len("boundary="):]
	return None

def parse_multipart_form_data(post_data, boundary):
	parts = post_data.split("--" + boundary)
	form_data = {}
	for part in parts:
		part = part.strip()
		if part and part != "--":
			headers, _, body = part.partition("\r\n\r\n")
			header_lines = headers.split("\r\n")
			name = None
			for header in header_lines:
				if header.startswith("Content-Disposition:"):
					disposition_parts = header.split(";")
					for disp_part in disposition_parts:
						disp_part = disp_part.strip()
						if disp_part.startswith("name="):
							name = disp_part[len("name="):].strip('"')
							break
			if name:
				form_data[name] = body.strip()
	return form_data

def handle_post():
	print("Content-Type: text/html; charset=UTF-8")
	print("Status: 200 OK")
	print()
	print("<!doctype html>")
	print("<html><head><meta charset='UTF-8'><title>CGI POST</title></head><body>")
	print("<h1>Hello from CGI POST!</h1>")
	print(os.environ.get("CONTENT_TYPE", ""))
	print("<h2>Received POST Data from Standard Input</h2>")
	try:
		content_length = int(os.environ.get("CONTENT_LENGTH", 0))
		if content_length > 0:
			post_data = os.sys.stdin.read(content_length)
			print(f"<pre>{post_data}</pre>")
			boundary = search_boundary(os.environ.get("CONTENT_TYPE"))
			if boundary:
				form_data = parse_multipart_form_data(post_data, boundary)
				for name, value in form_data.items():
					print(f"<h2>{name}</h2>")
					print(f"<pre>{value}</pre>")
		else:
			print("<p>No POST data received.</p>")
	except (ValueError, OSError):
		print("<p>Error reading POST data.</p>")
	print("</body></html>")
if __name__ == "__main__":
	handle_post()
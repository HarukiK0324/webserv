# !/usr/bin/env python3
import os


def get_query_string():
	query_string = os.environ.get("QUERY_STRING", "")
	print("Content-Type: text/html	; charset=UTF-8")
	print("Status: 200 OK")
	print()
	print("<!doctype html>")
	print("<html><head><meta charset='UTF-8'><title>CGI GetQuery</title></head><body>")
	print("<h1>Hello from CGI GetQuery!</h1>")
	print("<h2>Query String</h2>")
	print(query_string)	
	print("</body></html>")
if __name__ == "__main__":
	get_query_string()

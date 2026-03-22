from pathlib import Path

BASE_CONF_SERVER = Path("ok/full.conf")
BASE_CONF_LOCATION = Path("ok/full_location.conf")
BASE_SYNTAX_SERVER = Path("ok/syntax_server.conf")
BASE_SYNTAX_LOCATION = Path("ok/syntax_location.conf")
OUT_DIR = Path("ng")

OUT_DIR.mkdir(parents=True, exist_ok=True)

def write(name: str, content: str):
    path = OUT_DIR / name
    path.write_text(content)
    print(f"generated: {path}")

# base 読み込み
base_server = BASE_CONF_SERVER.read_text()
base_location = BASE_CONF_LOCATION.read_text()
base_syntax_server = BASE_SYNTAX_SERVER.read_text()
base_syntax_location = BASE_SYNTAX_LOCATION.read_text()


# ==============================
# NG ケース定義（1つずつ壊す）
# ==============================

NG_CASE_SERVER = [
    # ===== server =====
    ("server_unknown_directive.conf",
     "server {\n",
     "server {\n    foo bar;\n"),

    ("missing_semicolon.conf",
     "listen 8080;\n",
     "listen 8080\n"),

    ("listen_multi_args.conf",
     "listen 8080;",
     "listen localhost localhost2;"),

    ("listen_colon_only.conf",
     "listen 8080;",
     "listen localhost:;"),

    ("listen_alpha.conf",
     "listen 8080;",
     "listen abc;"),

    ("listen_privileged.conf",
     "listen 8080;",
     "listen 80;"),

    ("root_multi.conf",
     "root /var/www/html;",
     "root /a /b;"),

    ("client_body_multi.conf",
     "client_max_body_size 1000;",
     "client_max_body_size 65 643;"),

    ("client_body_alpha.conf",
     "client_max_body_size 1000;",
     "client_max_body_size abc;"),

    ("client_body_negative.conf",
     "client_max_body_size 1000;",
     "client_max_body_size -1;"),

    ("client_body_huge.conf",
     "client_max_body_size 1000;",
     "client_max_body_size 999999999999;"),

    ("keepalive_multi.conf",
     "keepalive_timeout 10;",
     "keepalive_timeout 50 50;"),

    ("keepalive_alpha.conf",
     "keepalive_timeout 10;",
     "keepalive_timeout abc;"),

    ("keepalive_negative.conf",
     "keepalive_timeout 10;",
     "keepalive_timeout -5;"),

    ("error_page_missing.conf",
     "error_page 404 /404.html;",
     "error_page 404;"),

    ("error_page_no_slash.conf",
     "error_page 404 /404.html;",
     "error_page 404 error.html;"),

    ("error_page_alpha.conf",
     "error_page 404 /404.html;",
     "error_page abc /404.html;"),

    ("error_page_range.conf",
     "error_page 404 /404.html;",
     "error_page 999 /404.html;"),

    ("autoindex_multi.conf",
     "autoindex off;",
     "autoindex on off;"),

    ("autoindex_invalid.conf",
     "autoindex off;",
     "autoindex maybe;"),
]

NG_CASE_LOCATION = [
    # ===== location =====
    ("loc_unknown_directive.conf",
     "location / {\n",
     "location / {\n    foo bar;\n"),

    ("loc_root_multi.conf",
     "root /var/www/html;",
     "root /var/www/html /var/www/html2;"),

    ("loc_allow_put.conf",
     "allow_methods GET POST;",
     "allow_methods PUT;"),

    ("loc_cgi_one_arg.conf",
     "cgi_pass .py /usr/bin/python3;",
     "cgi_pass .py;"),

    ("loc_cgi_no_dot.conf",
     "cgi_pass .py /usr/bin/python3;",
     "cgi_pass py /usr/bin/python3;"),

    ("loc_upload_multi.conf",
     "upload_path /var/www/upload/tmp;",
     "upload_path /var/www/upload/tmp /var/www/upload/tmp2;"),

    ("loc_return_missing.conf",
     "return 301 /new;",
     "return 301;"),

    ("loc_return_alpha.conf",
     "return 301 /new;",
     "return abc /x;"),

    ("loc_return_range.conf",
     "return 301 /new;",
     "return 999 /x;"),

    ("loc_autoindex_multi.conf",
     "autoindex off;",
     "autoindex on off;"),

    ("loc_autoindex_invalid.conf",
     "autoindex off;",
     "autoindex maybe;"),
]

NG_CASE_SYNTAX_SERVER = [
    # Expected 'server'
    (
        "syntax_expected_server.conf",
        "server {\n    listen 8080;\n}\n",
        "location / {\n    root /var/www;\n}\n"
    ),

    # Missing '{' after 'server'
    (
        "syntax_missing_lbrace_after_server.conf",
        "server {\n    listen 8080;\n}\n",
        "server\n    listen 8080;\n"
    ),

    # Directive key expected
    (
        "syntax_directive_key_expected.conf",
        "server {\n    listen 8080;\n}\n",
        "server {\n    ;\n}\n"
    ),

    # Directive requires at least one value
    (
        "syntax_directive_requires_value.conf",
        "server {\n    listen 8080;\n}\n",
        "server {\n    root;\n}\n"
    ),

    # Missing ';' at end of directive
    (
        "syntax_missing_semicolon.conf",
        "server {\n    listen 8080;\n}\n",
        "server {\n    listen 8080\n}\n"
    ),
]


NG_CASE_SYNTAX_LOCATION = [
    # Location block requires a path
    (
        "syntax_location_requires_path.conf",
        "location / {\n        root /var/www/html;\n    }\n",
        "location {\n        root /var/www/html;\n    }\n"
    ),

    # Location block must start with '{'
    (
        "syntax_location_missing_lbrace.conf",
        "location / {\n        root /var/www/html;\n    }\n",
        "location / root /var/www/html;\n"
    ),

    # Unexpected end of file in location block
    (
        "syntax_location_unexpected_eof.conf",
        "location / {\n        root /var/www/html;\n    }\n",
        "location / {\n        root /var/www/html;\n"
    ),
]

for name, old, new in NG_CASE_SYNTAX_SERVER:
    write(name, base_syntax_server.replace(old, new))

for name, old, new in NG_CASE_SYNTAX_LOCATION:
    write(name, base_syntax_location.replace(old, new))


# for name, old, new in NG_CASE_SERVER:
#     write(name, base_server.replace(old, new))

# for name, old, new in NG_CASE_LOCATION:
#     write(name, base_location.replace(old, new))

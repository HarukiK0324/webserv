from pathlib import Path

CONF_DIR = Path("ng")

SYNTAX_PREFIX = "ConfigParser_"
SEMANTIC_PREFIX = "ConfigBuilder_"

for conf in CONF_DIR.glob("*.conf"):
    name = conf.name

    # すでに prefix が付いているものはスキップ
    if name.startswith(SYNTAX_PREFIX) or name.startswith(SEMANTIC_PREFIX):
        continue

    if name.startswith("syntax"):
        new_name = SYNTAX_PREFIX + name
    else:
        new_name = SEMANTIC_PREFIX + name

    new_path = conf.with_name(new_name)

    print(f"{conf.name} -> {new_name}")
    conf.rename(new_path)

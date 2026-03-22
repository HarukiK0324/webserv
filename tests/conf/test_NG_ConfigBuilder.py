import subprocess
from pathlib import Path
import pytest

ROOT = Path(__file__).resolve().parents[2]  # プロジェクトルート
WEBSERV = ROOT / "conf_test"
NG_CONF_DIR = ROOT / "tests/conf/ng"

@pytest.mark.parametrize(
    "conf",
    list(NG_CONF_DIR.glob("ConfigBuilder_*.conf")), 
    ids=lambda p: p.name
)
def test_ngconf(conf):
    result = subprocess.run(
        [str(WEBSERV), str(conf)],
    )
    assert result.returncode != 0

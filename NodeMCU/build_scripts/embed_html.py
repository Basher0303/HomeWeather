import sys
from pathlib import Path

if "__file__" in globals():
    ROOT = Path(__file__).resolve().parents[1]
else:
    ROOT = Path.cwd()

SRC_DIR = ROOT / "src"
TEMPLATE_PATH = SRC_DIR / "index.html"
OUT_DIR = ROOT / "src" / "generated"
OUT_PATH = OUT_DIR / "index_html.h"


def escape_cpp_string(value: str) -> str:
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\r", "\\r")
        .replace("\n", "\\n")
    )


def main() -> None:
    if not TEMPLATE_PATH.exists():
        raise FileNotFoundError(f"HTML template not found: {TEMPLATE_PATH}")

    html = TEMPLATE_PATH.read_text(encoding="utf-8")
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    escaped_html = escape_cpp_string(html)
    header = f'''#pragma once

namespace generated {{
static const char INDEX_HTML[] = "{escaped_html}";
}}
'''

    OUT_PATH.write_text(header, encoding="utf-8")


try:
    # Выполняем генерацию при импорте (PlatformIO выполняет скрипт как модуль)
    main()
except Exception as e:
    # Печатаем ошибку, чтобы логи сборки содержали подсказку
    import traceback

    print(f"embed_html.py: generation failed: {e}")
    traceback.print_exc()

#!/usr/bin/env python3
"""Require C++17 nested syntax for named namespaces in Astron sources."""

from pathlib import Path
import re
import sys


NAMESPACE = re.compile(
    r"\b(?:inline\s+)?namespace\s*(?:(?P<name>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*)?\{"
)


def without_comments_and_literals(source: str) -> str:
    """Replace non-code with spaces while retaining character positions and lines."""
    result = list(source)
    index = 0

    def blank(start: int, end: int) -> None:
        for position in range(start, end):
            if result[position] != "\n":
                result[position] = " "
    while index < len(source):
        start = index
        if source.startswith("//", index):
            end = source.find("\n", index)
            index = len(source) if end < 0 else end
        elif source.startswith("/*", index):
            end = source.find("*/", index + 2)
            index = len(source) if end < 0 else end + 2
        elif source.startswith('R"', index):
            opening = source.find("(", index + 2)
            if opening < 0:
                index += 1
                continue
            delimiter = source[index + 2 : opening]
            closing = source.find(")" + delimiter + '"', opening + 1)
            index = len(source) if closing < 0 else closing + len(delimiter) + 2
        elif source[index] in {'"', "'"}:
            quote = source[index]
            index += 1
            while index < len(source):
                if source[index] == "\\":
                    index += 2
                elif source[index] == quote:
                    index += 1
                    break
                else:
                    index += 1
        else:
            index += 1
            continue
        blank(start, min(index, len(source)))

    return "".join(result)


def violations(path: Path) -> list[str]:
    source = path.read_text(encoding="utf-8")
    code = without_comments_and_literals(source)
    openings = {match.end() - 1: match.group("name") for match in NAMESPACE.finditer(code)}
    stack: list[bool] = []
    errors: list[str] = []

    for position, character in enumerate(code):
        if character == "{":
            if position in openings:
                name = openings[position]
                if name is not None and any(stack):
                    line = source.count("\n", 0, position) + 1
                    errors.append(
                        f"{path}:{line}: named namespace '{name}' is nested; "
                        "use a top-level 'namespace outer::inner' declaration"
                    )
                stack.append(True)
            else:
                stack.append(False)
        elif character == "}" and stack:
            stack.pop()

    return errors


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    source_paths = sorted((root / "src").rglob("*.cpp"))
    source_paths += sorted((root / "src").rglob("*.h"))
    errors = [error for path in source_paths for error in violations(path)]
    for error in errors:
        print(error, file=sys.stderr)
    if errors:
        return 1
    print(f"Namespace style OK ({len(source_paths)} files)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

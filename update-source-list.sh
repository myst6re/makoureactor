#!/bin/sh

# Updates the PROJECT_SOURCES variable in CMakeLists.txt

CMAKELISTS="$(dirname "$0")"/CMakeLists.txt

(
    cat "$CMAKELISTS" | \
    tr '\n' '\f' | \
    sed -r 's|set\(PROJECT_SOURCES[^)]*\)|set(PROJECT_SOURCES'"$(printf '\f')$(find -s src -type f '(' -name '*.cpp' -o -name '*.h' ')' -exec echo '    "'{}'"' ';' | sed 's/|/\\|/' | tr '\n' '\f')"')|' | \
    tr '\f' '\n' > "$CMAKELISTS".tmp && \
    mv "$CMAKELISTS".tmp "$CMAKELISTS"
) || rm -f "$CMAKELISTS".tmp

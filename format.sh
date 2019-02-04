#!/bin/sh

set -euo pipefail

HEADERS=$(find . -path ./.git -prune -o -name '*.h' -print)
SOURCES=$(find . -path ./.git -prune -o -name '*.cpp' -print)

for FILENAME in $HEADERS $SOURCES; do
  COMMAND="clang-format -i -style='{ BasedOnStyle: LLVM, ColumnLimit: 120 }' '$FILENAME'"
  echo "> $COMMAND"
  eval "$COMMAND"
done

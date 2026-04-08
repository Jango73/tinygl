#!/usr/bin/env bash

set -eu

ScriptDir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
ProjectRoot="$(CDPATH='' cd -- "${ScriptDir}/../.." && pwd)"

find "${ProjectRoot}" -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +

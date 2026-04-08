#!/usr/bin/env bash

set -eu

readonly SCRIPT_DIR="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly PROJECT_ROOT="$(CDPATH='' cd -- "${SCRIPT_DIR}/../.." && pwd)"
readonly DEFAULT_BUILD_DIR="${PROJECT_ROOT}/build"

# Returns the configured build directory, or the default one.
getBuildDir() {
    printf '%s\n' "${TINYGL_BUILD_DIR:-${DEFAULT_BUILD_DIR}}"
}

# Configures the CMake build directory for TinyGL.
configureTinyGl() {
    cmake -S "${PROJECT_ROOT}" -B "$(getBuildDir)"
}

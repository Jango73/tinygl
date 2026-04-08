#!/usr/bin/env bash

set -eu

ScriptDir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"

# shellcheck source=./common.sh
. "${ScriptDir}/common.sh"

BuildDir="$(getBuildDir)"
InstallPrefix="$(mktemp -d /tmp/tinygl-phase9-install.XXXXXX)"
ExternalBinary="$(mktemp /tmp/tinygl-phase9-linkage.XXXXXX)"
ExportList="$(mktemp /tmp/tinygl-phase9-exports.XXXXXX)"

cleanup() {
    rm -f "${ExternalBinary}" "${ExportList}"
    rm -rf "${InstallPrefix}"
}

trap cleanup EXIT

"${ScriptDir}/build"

if [ ! -f "${BuildDir}/libtinygl.so" ]; then
    printf 'phase9: missing shared library: %s/libtinygl.so\n' "${BuildDir}" >&2
    exit 1
fi

nm -D --defined-only "${BuildDir}/libtinygl.so" > "${ExportList}"

if ! grep -q ' tinyglCreateContext$' "${ExportList}"; then
    printf 'phase9: missing exported symbol tinyglCreateContext\n' >&2
    exit 1
fi

if ! grep -q ' tinyglPresent$' "${ExportList}"; then
    printf 'phase9: missing exported symbol tinyglPresent\n' >&2
    exit 1
fi

if ! grep -q ' glBegin$' "${ExportList}"; then
    printf 'phase9: missing exported symbol glBegin\n' >&2
    exit 1
fi

if ! grep -q ' glGetError$' "${ExportList}"; then
    printf 'phase9: missing exported symbol glGetError\n' >&2
    exit 1
fi

"${ScriptDir}/install" "${InstallPrefix}"

cc \
    "${PROJECT_ROOT}/tests/external/external_linkage_smoke.c" \
    -I"${InstallPrefix}/include" \
    -L"${InstallPrefix}/lib" \
    -Wl,-rpath,"${InstallPrefix}/lib" \
    -ltinygl \
    -lm \
    -o "${ExternalBinary}"

"${ExternalBinary}"

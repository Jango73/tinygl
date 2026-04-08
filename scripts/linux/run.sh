#!/usr/bin/env bash

set -eu

ScriptDir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"

# shellcheck source=./common.sh
. "${ScriptDir}/common.sh"

BuildDir="$(getBuildDir)"

"${ScriptDir}/build"
exec "${BuildDir}/tinygl_demo_sdl2" "$@"

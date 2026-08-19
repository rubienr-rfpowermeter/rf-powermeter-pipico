#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BIN_NAME="rf_probe.elf"

if ! PICOTOOL_BIN=$(command -v picotool); then
  PICOTOOL_BIN="${SCRIPT_DIR}/../build/_deps/picotool/picotool"
fi

pushd "${SCRIPT_DIR}/../build" \
&& "${PICOTOOL_BIN}" info -a "${BIN_NAME}" \
&& popd

#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BIN_NAME=rf_probe.elf

pushd ${SCRIPT_DIR}/../build \
&&  _deps/picotool/picotool info -a ${BIN_NAME} -a \
&& popd

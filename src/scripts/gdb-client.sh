#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# MCU=rp2040
MCU=rp2350

FILE="rf_probe.elf"
REMOTE="localhost:3333"
# EXTRA_ARGS="--tui"
EXTRA_ARGS=""

pushd ${SCRIPT_DIR}/../build
gdb-multiarch $FILE $EXTRA_ARGS --eval-command="target remote ${REMOTE}"
popd

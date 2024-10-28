#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROGRAM="rf_probe.elf"
#BOARD="rp2040"
BOARD="rp2350"


pushd ${SCRIPT_DIR}/../build \
&& time \
openocd --debug=2 \
        --search ${SCRIPT_DIR}/openocd \
        --file ${BOARD}-cmsis-dap.cfg \
        --command "program ${PROGRAM} verify reset exit" \
&& popd

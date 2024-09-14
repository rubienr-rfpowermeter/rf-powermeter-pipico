#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROGRAM="rf_probe.elf"

pushd ${SCRIPT_DIR}/../build \
&& time \
openocd --debug=2 \
        --search  ${SCRIPT_DIR}/openocd \
        --file rp2040-cmsis-dap.cfg \
        --command "program ${PROGRAM} verify reset exit" \
&& popd

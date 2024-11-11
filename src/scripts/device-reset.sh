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
        --command "init; reset halt; rp2350.dap.core1 arp_reset assert 0; rp2350.dap.core0 arp_reset assert 0; exit" \
&& popd

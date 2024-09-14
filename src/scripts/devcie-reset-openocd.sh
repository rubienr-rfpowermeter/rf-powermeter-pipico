#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

openocd --debug=2 \
        --search  ${SCRIPT_DIR}/openocd \
        --file    rp2040-cmsis-dap.cfg \
        --command "init; reset halt; rp2040.core1 arp_reset assert 0; rp2040.core0 arp_reset assert 0; exit"

#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

openocd -f /usr/share/openocd/scripts/interface/cmsis-dap.cfg \
        -f /usr/share/openocd/scripts/target/rp2040.cfg \
        -c "init; reset halt; rp2040.core1 arp_reset assert 0; rp2040.core0 arp_reset assert 0; exit"

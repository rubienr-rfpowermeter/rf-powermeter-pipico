#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
NUM_CORES=`nproc`
PROGRAM="rf_probe.elf"
SWD_SPEED_KHZ="20000"

pushd ${SCRIPT_DIR}/../build \
&& time openocd -f /usr/share/openocd/scripts/interface/cmsis-dap.cfg \
        -f /usr/share/openocd/scripts/target/rp2040.cfg \
        -c "adapter speed ${SWD_SPEED_KHZ}" \
        -c "program ${PROGRAM} verify reset exit" \
&& popd

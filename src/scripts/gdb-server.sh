#!/usr/bin/env bash
# Define this script as GDB-server command in "Embedded GDB server" run configuration (CLion).

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# MCU=rp2040
MCU=rp2350

pushd ${SCRIPT_DIR}/../build
openocd --search  ${SCRIPT_DIR}/openocd \
        --file ${MCU}-cmsis-dap.cfg
popd

#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd ${SCRIPT_DIR}/../build
openocd --search  ${SCRIPT_DIR}/openocd \
        --file rp2040-cmsis-dap.cfg
popd

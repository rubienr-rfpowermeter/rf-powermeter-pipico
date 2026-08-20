#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROGRAM="rf_probe.elf"

OPENOCD_ARGS=(
  --debug=2
  --file interface/cmsis-dap.cfg
  --command "adapter speed 20000"
  --file target/rp2350.cfg
  --command "program ${PROGRAM} verify reset exit"
)

pushd "${SCRIPT_DIR}/../build" \
&& time \
openocd "${OPENOCD_ARGS[@]}" \
&& popd

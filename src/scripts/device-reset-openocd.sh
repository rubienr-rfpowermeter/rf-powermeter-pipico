#!/usr/bin/env bash

set -euo pipefail

OPENOCD_ARGS=(
  --debug=2
  --file interface/cmsis-dap.cfg
  --command "adapter speed 20000"
  --file target/rp2350.cfg
  --command "init; cold_reset; sleep 100; shutdown"
)

exec openocd "${OPENOCD_ARGS[@]}"

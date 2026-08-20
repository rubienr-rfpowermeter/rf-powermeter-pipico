#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd "${SCRIPT_DIR}/../build" || exit 1

exec openocd \
  --file interface/cmsis-dap.cfg \
  --command "adapter speed 20000" \
  --file target/rp2350.cfg

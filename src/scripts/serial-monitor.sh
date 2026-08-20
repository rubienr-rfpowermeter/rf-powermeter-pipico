#!/usr/bin/env bash

set -euo pipefail

SERIAL_PORT=${1:-/dev/ttyACM0}

BAUD_RATE=115200
DATA_BITS=8
PARITY="none"
STOP_BITS=1

picocom "${SERIAL_PORT}" \
  --baud "${BAUD_RATE}" \
  --databits="${DATA_BITS}" \
  --parity="${PARITY}" \
  --stopbits "${STOP_BITS}" \
  --echo

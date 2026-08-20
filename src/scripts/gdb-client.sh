#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

FILE="rf_probe.elf"
REMOTE="localhost:3333"

if command -v gdb-multiarch &> /dev/null; then
  GDB=gdb-multiarch
elif command -v gdb &> /dev/null; then
  GDB=gdb
else
  echo "Neither gdb-multiarch nor gdb is installed." >&2
  exit 1
fi

cd "${SCRIPT_DIR}/../build" || exit 1

exec "${GDB}" "${FILE}" \
  --eval-command="target extended-remote ${REMOTE}" \
  "$@"

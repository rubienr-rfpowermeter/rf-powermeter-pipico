#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [[ -z "${PICO_SDK_PATH:-}" ]]; then
  echo "PICO_SDK_PATH is not set." >&2
  echo "Set it to the Pico SDK directory before running this script." >&2
  exit 1
fi

if [[ ! -f "${PICO_SDK_PATH}/pico_sdk_init.cmake" ]]; then
  echo "PICO_SDK_PATH does not point to a Pico SDK: ${PICO_SDK_PATH}" >&2
  exit 1
fi

pushd "${SCRIPT_DIR}/.." \
&& rm -drf build \
&& cmake -GNinja \
         -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
         -DCMAKE_BUILD_TYPE:STRING=Debug \
         -DPICO_SDK_PATH:PATH="${PICO_SDK_PATH}" \
         -S ./ -B ./build \
&& popd

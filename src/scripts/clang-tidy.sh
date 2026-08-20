#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SOURCE_DIR=$( cd -- "${SCRIPT_DIR}/.." && pwd )
BUILD_DIR=${1:-"${SOURCE_DIR}/build"}

if ! command -v clang-tidy &> /dev/null; then
  echo "clang-tidy is not installed or not available in PATH." >&2
  exit 1
fi

if ! command -v arm-none-eabi-g++ &> /dev/null; then
  echo "arm-none-eabi-g++ is not installed or not available in PATH." >&2
  exit 1
fi

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
  echo "Compilation database not found: ${BUILD_DIR}/compile_commands.json" >&2
  echo "Configure the firmware with src/scripts/cmake.sh first." >&2
  exit 1
fi

mapfile -d '' -t SOURCE_FILES < <(
  find "${SOURCE_DIR}/modules" -type f \( -name '*.c' -o -name '*.cpp' \) -print0
  printf '%s\0' "${SOURCE_DIR}/main.cpp"
)

if (( ${#SOURCE_FILES[@]} == 0 )); then
  echo "No project-owned firmware source files found." >&2
  exit 1
fi

printf 'Analyzing %d project-owned firmware source files...\n' "${#SOURCE_FILES[@]}"

# Clang-Tidy reads the cross-compiler command line from compile_commands.json,
# but it cannot infer GCC's implicit C++ standard-library include directories.
mapfile -t GCC_SYSTEM_INCLUDE_DIRS < <(
  arm-none-eabi-g++ -E -x c++ -v /dev/null 2>&1 \
    | awk '
        /^#include <\.\.\.> search starts here:/ { emit = 1; next }
        /^End of search list\./ { emit = 0 }
        emit { sub(/^ /, ""); print }
      '
)

CLANG_TIDY_EXTRA_ARGS=()
for include_dir in "${GCC_SYSTEM_INCLUDE_DIRS[@]}"; do
  CLANG_TIDY_EXTRA_ARGS+=("--extra-arg-before=-isystem${include_dir}")
done

clang-tidy \
  --config-file="${SOURCE_DIR}/.clang-tidy" \
  --quiet \
  -p="${BUILD_DIR}" \
  "${CLANG_TIDY_EXTRA_ARGS[@]}" \
  "${SOURCE_FILES[@]}"

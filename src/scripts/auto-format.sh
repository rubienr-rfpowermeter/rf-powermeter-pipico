#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SOURCE_DIR=$( cd -- "${SCRIPT_DIR}/.." && pwd )
MAX_SOURCE_FILE_SIZE=1M
MODE=${1:-format}

if (( $# > 1 )); then
  echo "Usage: $0 [--check]" >&2
  exit 2
fi

case "${MODE}" in
  format)
    CLANG_FORMAT_ARGS=(--style=file -i)
    ACTION="Formatting"
    RESULT="Formatted"
    ;;
  --check)
    CLANG_FORMAT_ARGS=(--style=file --dry-run --Werror)
    ACTION="Checking the formatting of"
    RESULT="Formatting is correct for"
    ;;
  *)
    echo "Usage: $0 [--check]" >&2
    exit 2
    ;;
esac

if ! command -v clang-format &> /dev/null; then
  echo "clang-format is not installed or not available in PATH." >&2
  exit 1
fi

if ! command -v parallel &> /dev/null; then
  echo "GNU Parallel is not installed or not available in PATH." >&2
  exit 1
fi

mapfile -d '' -t SOURCE_FILES < <(
  find "${SOURCE_DIR}" \
    -type d \( \
      -name build -o \
      -name fonts -o \
      -name generated -o \
      -name gitmodules \
    \) -prune -o \
    -type f \( \
      -name '*.c' -o \
      -name '*.cc' -o \
      -name '*.cpp' -o \
      -name '*.cxx' -o \
      -name '*.h' -o \
      -name '*.hh' -o \
      -name '*.hpp' -o \
      -name '*.hxx' \
    \) ! -name 'lv_conf.h' ! -size "+${MAX_SOURCE_FILE_SIZE}" -print0
)

if (( ${#SOURCE_FILES[@]} == 0 )); then
  echo "No C/C++ source files found below ${SOURCE_DIR}." >&2
  exit 1
fi

printf '%s %d C/C++ source files...\n' "${ACTION}" "${#SOURCE_FILES[@]}"

PARALLEL_OUTPUT_ARGS=(--verbose)
if [[ -t 2 ]] && ( : > /dev/tty ) 2>/dev/null; then
  PARALLEL_OUTPUT_ARGS=(--bar)
fi

printf '%s\0' "${SOURCE_FILES[@]}" \
  | parallel --will-cite "${PARALLEL_OUTPUT_ARGS[@]}" --line-buffer \
      --null --no-run-if-empty --halt soon,fail=1 \
      clang-format "${CLANG_FORMAT_ARGS[@]}" {}

printf '%s %d C/C++ source files.\n' "${RESULT}" "${#SOURCE_FILES[@]}"

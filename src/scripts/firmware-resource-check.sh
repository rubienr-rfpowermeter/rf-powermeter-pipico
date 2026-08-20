#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BUILD_DIR=${1:-"${SCRIPT_DIR}/../build"}
PROGRAM_NAME=${PROGRAM_NAME:-rf_probe}

# These limits leave approximately 10-12% headroom over the baseline measured
# when this check was introduced. Override them in the environment when an
# intentional resource increase has been reviewed.
MAX_FLASH_BYTES=${MAX_FLASH_BYTES:-614400}
MAX_MAIN_SRAM_BYTES=${MAX_MAIN_SRAM_BYTES:-307200}
MAX_PROJECT_STACK_FRAME_BYTES=${MAX_PROJECT_STACK_FRAME_BYTES:-256}

ELF_FILE="${BUILD_DIR}/${PROGRAM_NAME}.elf"
BIN_FILE="${BUILD_DIR}/${PROGRAM_NAME}.bin"

for command_name in arm-none-eabi-size find awk sort stat; do
  if ! command -v "${command_name}" &> /dev/null; then
    echo "Required command is not available in PATH: ${command_name}" >&2
    exit 1
  fi
done

for required_file in "${ELF_FILE}" "${BIN_FILE}"; do
  if [[ ! -f "${required_file}" ]]; then
    echo "Firmware output not found: ${required_file}" >&2
    exit 1
  fi
done

FLASH_BYTES=$(stat --format='%s' "${BIN_FILE}")

# RP2350 provides 512 KiB of main SRAM at 0x20000000, followed by 8 KiB of
# scratch SRAM. Sum allocated sections by their VMA as reported by GNU size.
read -r MAIN_SRAM_BYTES SCRATCH_SRAM_BYTES < <(
  arm-none-eabi-size --format=sysv "${ELF_FILE}" \
    | awk '
        $2 ~ /^[0-9]+$/ && $3 ~ /^[0-9]+$/ {
          if ($3 >= 536870912 && $3 < 537395200) main_sram += $2
          if ($3 >= 537395200 && $3 < 537403392) scratch_sram += $2
        }
        END { print main_sram + 0, scratch_sram + 0 }
      '
)

LARGEST_STACK_RECORD=$(
  find "${BUILD_DIR}" -type f -name '*.su' -print0 \
    | while IFS= read -r -d '' usage_file; do
        awk -F '\t' '
          NF >= 3 && $2 ~ /^[0-9]+$/ {
            source = $1
            symbol = $1
            if (match($1, /^.*:[0-9]+:[0-9]+:/)) {
              source = substr($1, 1, RLENGTH - 1)
              symbol = substr($1, RLENGTH + 1)
            }
            if (source !~ /\/pico-sdk\// &&
                source !~ /\/arm-none-eabi\// &&
                source !~ /\/src\/gitmodules\//) {
              printf "%s\t%s\t%s\n", $2, source, symbol
            }
          }
        ' "${usage_file}"
      done \
    | sort -t $'\t' -k1,1nr \
    | awk 'NR == 1 { print; exit }'
)

if [[ -z "${LARGEST_STACK_RECORD}" ]]; then
  echo "No project-owned stack-usage records found below ${BUILD_DIR}." >&2
  exit 1
fi

IFS=$'\t' read -r LARGEST_STACK_FRAME_BYTES LARGEST_STACK_SOURCE LARGEST_STACK_SYMBOL \
  <<< "${LARGEST_STACK_RECORD}"

percentage() {
  awk -v used="$1" -v limit="$2" 'BEGIN { printf "%.1f%%", used * 100 / limit }'
}

printf '%-28s %10s %10s %10s %8s\n' RESOURCE USED LIMIT HEADROOM USED_PCT
printf '%-28s %10s %10s %10s %8s\n' ---------------------------- ---------- ---------- ---------- --------
printf '%-28s %10d %10d %10d %8s\n' \
  "Flash image (bytes)" "${FLASH_BYTES}" "${MAX_FLASH_BYTES}" \
  "$(( MAX_FLASH_BYTES - FLASH_BYTES ))" "$(percentage "${FLASH_BYTES}" "${MAX_FLASH_BYTES}")"
printf '%-28s %10d %10d %10d %8s\n' \
  "Main SRAM static (bytes)" "${MAIN_SRAM_BYTES}" "${MAX_MAIN_SRAM_BYTES}" \
  "$(( MAX_MAIN_SRAM_BYTES - MAIN_SRAM_BYTES ))" "$(percentage "${MAIN_SRAM_BYTES}" "${MAX_MAIN_SRAM_BYTES}")"
printf '%-28s %10d %10d %10d %8s\n' \
  "Project stack frame (bytes)" "${LARGEST_STACK_FRAME_BYTES}" "${MAX_PROJECT_STACK_FRAME_BYTES}" \
  "$(( MAX_PROJECT_STACK_FRAME_BYTES - LARGEST_STACK_FRAME_BYTES ))" \
  "$(percentage "${LARGEST_STACK_FRAME_BYTES}" "${MAX_PROJECT_STACK_FRAME_BYTES}")"
printf '%-28s %10d %10s %10s %8s\n' "Scratch SRAM reserved" "${SCRATCH_SRAM_BYTES}" 8192 0 100.0%

printf '\nLargest project-owned stack frame:\n%s\n%s\n' \
  "${LARGEST_STACK_SOURCE}" "${LARGEST_STACK_SYMBOL}"

FAILED=0

if (( FLASH_BYTES > MAX_FLASH_BYTES )); then
  echo "Flash image exceeds its limit by $(( FLASH_BYTES - MAX_FLASH_BYTES )) bytes." >&2
  FAILED=1
fi

if (( MAIN_SRAM_BYTES > MAX_MAIN_SRAM_BYTES )); then
  echo "Static main SRAM usage exceeds its limit by $(( MAIN_SRAM_BYTES - MAX_MAIN_SRAM_BYTES )) bytes." >&2
  FAILED=1
fi

if (( LARGEST_STACK_FRAME_BYTES > MAX_PROJECT_STACK_FRAME_BYTES )); then
  echo "Largest project-owned stack frame exceeds its limit by $(( LARGEST_STACK_FRAME_BYTES - MAX_PROJECT_STACK_FRAME_BYTES )) bytes." >&2
  FAILED=1
fi

exit "${FAILED}"

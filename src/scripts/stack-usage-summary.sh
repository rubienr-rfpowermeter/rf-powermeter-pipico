#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SEARCH_DIR=${1:-"${SCRIPT_DIR}/../build"}

if [[ ! -d "${SEARCH_DIR}" ]]; then
  echo "Build directory not found: ${SEARCH_DIR}" >&2
  exit 1
fi

mapfile -d '' -t USAGE_FILES < <(find "${SEARCH_DIR}" -type f -name '*.su' -print0)

if (( ${#USAGE_FILES[@]} == 0 )); then
  echo "No .su files found below ${SEARCH_DIR}" >&2
  exit 1
fi

for usage_file in "${USAGE_FILES[@]}"; do
  awk -F '\t' '
    NF >= 3 && $2 ~ /^[0-9]+$/ {
      source = $1
      symbol = $1

      # GCC writes source-file:line:column:symbol in the first field.
      if (match($1, /^.*:[0-9]+:[0-9]+:/)) {
        source = substr($1, 1, RLENGTH - 1)
        symbol = substr($1, RLENGTH + 1)
      }

      printf "%s\t%s\t%s\t%s\n", $2, $3, symbol, source
    }
  ' "${usage_file}"
done \
  | sort -t $'\t' -k1,1nr -k2,2 -k3,3 -k4,4 -u \
  | awk -F '\t' '
      {
        bytes[NR] = $1
        usage[NR] = $2
        symbol[NR] = $3
        source[NR] = $4

        if (length($1) > bytes_width) bytes_width = length($1)
        if (length($2) > usage_width) usage_width = length($2)
        if (length($4) > source_width) source_width = length($4)
      }

      END {
        if (bytes_width < length("BYTES")) bytes_width = length("BYTES")
        if (usage_width < length("USAGE")) usage_width = length("USAGE")
        if (source_width < length("SOURCE")) source_width = length("SOURCE")

        printf "%*s  %-*s  %-*s  %s\n", bytes_width, "BYTES", usage_width, "USAGE", source_width, "SOURCE", "SYMBOL"
        for (i = 1; i <= bytes_width; i++) printf "-"
        printf "  "
        for (i = 1; i <= usage_width; i++) printf "-"
        printf "  "
        for (i = 1; i <= source_width; i++) printf "-"
        printf "  ------\n"

        for (row = 1; row <= NR; row++) {
          printf "%*s  %-*s  %-*s  %s\n", bytes_width, bytes[row], usage_width, usage[row], source_width, source[row], symbol[row]
        }
      }
    '

#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

sudo apt-get update

packages=(
  clang-format
  cmake
  gdb
  gdb-multiarch
  gcc-arm-none-eabi
  libnewlib-arm-none-eabi
  libstdc++-arm-none-eabi-newlib
  ninja-build
  openocd
  parallel
  picocom
)

if apt-cache show picotool &> /dev/null; then
  packages+=(picotool)
fi

sudo apt install --yes "${packages[@]}"

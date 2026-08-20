#!/usr/bin/env bash

set -euo pipefail

sudo apt-get update

packages=(
  clang-format
  clang-tidy
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
  shellcheck
)

if apt-cache show picotool &> /dev/null; then
  packages+=(picotool)
fi

sudo apt install --yes "${packages[@]}"

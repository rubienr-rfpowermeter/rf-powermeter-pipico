#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

sudo apt-get update

packages=(
  cmake
  gdb
  gdb-multiarch
  gcc-arm-none-eabi
  libnewlib-arm-none-eabi
  libstdc++-arm-none-eabi-newlib
  ninja-build
  openocd
  picocom
)

if apt-cache show picotool &> /dev/null; then
  packages+=(picotool)
fi

sudo apt install --yes "${packages[@]}"

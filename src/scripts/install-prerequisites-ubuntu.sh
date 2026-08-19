#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

packages=(
  cmake
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

sudo apt install "${packages[@]}"

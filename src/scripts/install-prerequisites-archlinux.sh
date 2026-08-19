#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

packages=(
  arm-none-eabi-gcc
  arm-none-eabi-newlib
  cmake
  ninja
  openocd
  picocom
)

sudo pacman -S --needed "${packages[@]}"


if command -v yay &> /dev/null; then
  aur_packages=(
    picotool
  )

  yay -S --needed "${aur_packages[@]}"
fi

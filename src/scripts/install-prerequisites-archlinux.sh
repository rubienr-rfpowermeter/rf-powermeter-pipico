#!/usr/bin/env bash

set -euo pipefail

packages=(
  arm-none-eabi-gcc
  arm-none-eabi-newlib
  base-devel
  clang
  cmake
  gdb
  ninja
  #openocd
  parallel
  picocom
  shellcheck
)

if (( EUID == 0 )); then
  pacman -S --noconfirm --needed "${packages[@]}"
else
  sudo pacman -S --noconfirm --needed "${packages[@]}"
fi


if command -v yay &> /dev/null; then
  aur_packages=(
    picotool
    openocd-git
  )

  yay -S --noconfirm --needed "${aur_packages[@]}"
fi

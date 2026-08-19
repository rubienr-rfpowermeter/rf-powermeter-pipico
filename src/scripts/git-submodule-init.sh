#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
REPOSITORY_DIR=$(git -C "${SCRIPT_DIR}" rev-parse --show-toplevel) || exit 1

git -C "${REPOSITORY_DIR}" submodule sync --recursive \
&& git -C "${REPOSITORY_DIR}" submodule update \
     --init \
     --recursive \
     --remote \
     --recommend-shallow \
     --single-branch \
     --verbose \
&& git -C "${REPOSITORY_DIR}" submodule status --recursive

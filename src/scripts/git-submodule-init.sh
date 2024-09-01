#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd ${SCRIPT_DIR}/.. \
&& git submodule update --init --recursive --recommend-shallow --single-branch --verbose \
&& git submodules status \
&& popd


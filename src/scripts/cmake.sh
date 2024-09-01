#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd ${SCRIPT_DIR}/.. \
&& mkdir build \
&& pushd build \
&& time cmake --no-warn-unused-cli \
        -DPICO_OPTIMIZED_DEBUG=1 \
        -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
        -DCMAKE_BUILD_TYPE:STRING=Debug \
        ../ \
&& popd \
&& popd

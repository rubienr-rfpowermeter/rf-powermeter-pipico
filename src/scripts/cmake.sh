#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd ${SCRIPT_DIR}/.. \
&& rm -drf build \
&& cmake --no-warn-unused-cli \
         -DPICO_OPTIMIZED_DEBUG=1 \
         -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
         -DCMAKE_BUILD_TYPE:STRING=Debug \
         -S ./ -B ./build \
&& popd

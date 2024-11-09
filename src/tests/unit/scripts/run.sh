#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
NUM_CORES=`nproc`

pushd ${SCRIPT_DIR} \
&& ./cmake.sh \
&& ./make.sh \
&& ./test.sh \
&& popd

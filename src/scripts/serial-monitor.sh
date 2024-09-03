#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

SERIAL_PORT="/dev/ttyACM0"
if [ "x$1" == "x"] ; then
  SERIAL_PORT="$1"
fi

BAUD_RATE=115200
DATA_BITS=8
PARITY="none"
STOP_BITS=1
EXTRA_ARGS="--echo"

picocom ${SERIAL_PORT} --baud ${BAUD_RATE} --databits=${DATA_BITS} --parity=${PARITY} --stopbits ${STOP_BITS} ${EXTRA_ARGS}

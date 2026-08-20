# Project scripts

The scripts in this directory resolve paths relative to their own location, so
they can normally be invoked from any working directory. The examples below
assume the repository root as the current directory.

## Setup

### `install-prerequisites-ubuntu.sh`

Installs the Ubuntu packages needed to configure, build, inspect, debug, and
upload the firmware. It installs `picotool` when that package is available in
the configured Ubuntu repositories.

```sh
src/scripts/install-prerequisites-ubuntu.sh
```

The script uses `sudo` and therefore may ask for a password.

### `install-prerequisites-archlinux.sh`

Installs the required packages from the official Arch Linux repositories. If
`yay` is available, it also installs the AUR packages `picotool` and
`openocd-git`.

```sh
src/scripts/install-prerequisites-archlinux.sh
```

Run it as a regular user. It uses `sudo` for `pacman` unless it is already
running as root; `yay` should not be run as root.

### `git-submodule-init.sh`

Synchronizes submodule URLs, initializes missing submodules, and recursively
checks out the revisions pinned by this repository. Calling it again updates
the submodules to any newer revisions recorded by the currently checked-out
superproject commit.

```sh
src/scripts/git-submodule-init.sh
```

It does not advance submodules to arbitrary upstream revisions.

## Configure and build

Before configuring, set `PICO_SDK_PATH` to a valid Pico SDK checkout:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
```

### `cmake.sh`

Deletes `src/build`, then configures a fresh Debug build with Ninja and exports
`compile_commands.json`. Because the existing build directory is removed, use
this when a clean reconfiguration is intended.

```sh
src/scripts/cmake.sh
```

### `make.sh`

Builds the already configured `src/build` directory using all available build
parallelism.

```sh
src/scripts/make.sh
```

### `cmake-make.sh`

Runs `cmake.sh` followed by `make.sh`, stopping if configuration fails. This is
a clean configure-and-build operation and therefore also deletes the previous
build directory.

```sh
src/scripts/cmake-make.sh
```

### `make-upload-openocd.sh`

Builds the configured project and, if the build succeeds, uploads it with
`upload-openocd.sh`.

```sh
src/scripts/make-upload-openocd.sh
```

## Device access and debugging

These commands expect a CMSIS-DAP probe connected to the RP2350 target unless
stated otherwise.

### `upload-openocd.sh`

Uses OpenOCD's installed CMSIS-DAP interface and RP2350 target configuration to
program, verify, reset, and start `src/build/rf_probe.elf`.

```sh
src/scripts/upload-openocd.sh
```

### `device-reset.sh`

Connects to the RP2350 through OpenOCD, halts and resets both cores, then exits
without programming firmware.

```sh
src/scripts/device-reset.sh
```

### `devcie-reset-openocd.sh`

Legacy RP2040 reset helper. The filename intentionally reflects its current
misspelling. It expects `scripts/openocd/rp2040-cmsis-dap.cfg`, which is not
present in the current tree, and is not the normal reset command for this
RP2350 project. Prefer `device-reset.sh`.

```sh
src/scripts/devcie-reset-openocd.sh
```

### `gdb-server.sh`

Starts OpenOCD for the RP2350 and exposes its core-0 GDB server on
`localhost:3333`. Leave it running in one terminal:

```sh
src/scripts/gdb-server.sh
```

### `gdb-client.sh`

Opens `src/build/rf_probe.elf` with `gdb-multiarch`, falling back to `gdb`, and
connects to the OpenOCD server at `localhost:3333`. Run it in a second terminal
after `gdb-server.sh`:

```sh
src/scripts/gdb-client.sh
```

Additional GDB command-line arguments are forwarded, for example:

```sh
src/scripts/gdb-client.sh --eval-command='monitor reset halt'
```

### `serial-monitor.sh`

Opens a 115200 baud, 8-N-1 `picocom` session with local echo enabled. It uses
`/dev/ttyACM0` by default; pass another serial device as the first argument.

```sh
src/scripts/serial-monitor.sh
src/scripts/serial-monitor.sh /dev/ttyACM1
```

Exit picocom with `Ctrl-A`, followed by `Ctrl-X`.

## Inspection and diagnostics

### `picotool-info.sh`

Prints all metadata from `src/build/rf_probe.elf`. It prefers `picotool` from
`PATH` and falls back to the copy built below `src/build/_deps/picotool`.

```sh
src/scripts/picotool-info.sh
```

### `stack-usage-summary.sh`

Finds GCC `.su` stack-usage files, removes duplicate records, and prints
functions in descending frame-size order. It searches `src/build` by default,
or a directory supplied as its first argument.

```sh
src/scripts/stack-usage-summary.sh
src/scripts/stack-usage-summary.sh /path/to/build
```

The `.su` files are produced because the firmware target is compiled with
`-fstack-usage`.

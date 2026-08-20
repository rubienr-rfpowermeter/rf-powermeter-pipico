# Pico RF Power Meter

Firmware for an RP2350-based RF power meter built around a Raspberry Pi Pico 2,
a Waveshare Pico-LCD-1.3, an AD8318 RF detector, and an AD7887 ADC.

Core 0 runs the LVGL user interface and core 1 performs continuous sampling.
Display transfers and ADC sampling use DMA.

```text
 RF probe                                    Pico 2                 LCD and controls
 ┌──────────────────────┐                  ┌────────┐  ── SPI1 ──▶  240 × 240 display
 │ RF input ──▶ AD8318  │                  │ RP2350 │  ◀─ GPIO ───  joystick and buttons
 │             │ analog │                  │        │
 │             ▼        │                  │        │  core 0: UI (LVGL)
 │            AD7887 ◀──┼──── SPI0 ───────▶│        │  core 1: sampling
 │ TEMP feedback ───────┼─ analog / ADC0 ─▶│        │
 └──────────────────────┘                  └────────┘
```

The diagram includes the intended analog probe-temperature feedback. The
current sampler carries a temperature value but does not yet acquire that ADC
input.

## Requirements

- Raspberry Pi Pico SDK 2.0.0 or newer, including its submodules
- CMake and Ninja
- `clang-format` and GNU Parallel for source formatting
- Arm GNU embedded toolchain and Newlib
- OpenOCD with RP2350 support for debug-probe uploads
- `picotool` for inspecting firmware metadata
- `picocom` for the serial monitor

The provided installer scripts install these dependencies on Ubuntu and Arch
Linux. On Arch Linux, `yay` is used for the `picotool` and `openocd-git` AUR
packages when it is available.

## Getting started

1. Clone the repository and initialize its pinned submodules:

   ```sh
   git clone git@github.com:rubienr/rf-powermeter-pipico.git
   cd rf-powermeter-pipico
   src/scripts/git-submodule-init.sh
   ```

   The LVGL submodule uses an SSH URL, so the GitHub SSH key for your account
   must be configured.

2. Install the build prerequisites:

   Ubuntu:

   ```sh
   src/scripts/install-prerequisites-ubuntu.sh
   ```

   Arch Linux:

   ```sh
   src/scripts/install-prerequisites-archlinux.sh
   ```

3. Clone the Pico SDK next to the project and export its absolute path. If it is
   already installed, only the `export` is necessary.

   ```sh
   cd ..
   git clone --branch 2.0.0 --recurse-submodules https://github.com/raspberrypi/pico-sdk.git
   export PICO_SDK_PATH="$PWD/pico-sdk"
   cd rf-powermeter-pipico
   ```

4. Configure a clean Debug build and compile the firmware:

   ```sh
   src/scripts/cmake-make.sh
   ```

   `cmake-make.sh` deletes and recreates `src/build`. For later incremental
   builds, use:

   ```sh
   src/scripts/make.sh
   ```

The main outputs are `src/build/rf_probe.elf` and
`src/build/rf_probe.uf2`. See the [scripts documentation](src/scripts/readme.md)
for all helper commands and their options.

## Uploading firmware

### CMSIS-DAP debug probe

Connect a Raspberry Pi Debug Probe, or another CMSIS-DAP probe, to the SWD
connector and run:

```sh
src/scripts/make-upload-openocd.sh
```

To upload an already compiled image without rebuilding it:

```sh
src/scripts/upload-openocd.sh
```

### BOOTSEL and UF2

Hold the Pico 2's BOOTSEL button while connecting USB, then copy
`src/build/rf_probe.uf2` to the mounted RP2350 mass-storage volume. The mount
path depends on the operating system and desktop environment.

## Serial output

The firmware uses UART0 at 115200 baud: GP0 is TX and GP1 is RX. When using the
serial bridge of a Raspberry Pi Debug Probe, monitor the default
`/dev/ttyACM0` device with:

```sh
src/scripts/serial-monitor.sh
```

Pass a different device as the first argument when necessary:

```sh
src/scripts/serial-monitor.sh /dev/ttyACM1
```

## Debugging

Start OpenOCD in one terminal and GDB in another:

```sh
src/scripts/gdb-server.sh
```

```sh
src/scripts/gdb-client.sh
```

The GDB server listens on `localhost:3333` and the client loads
`src/build/rf_probe.elf`.

## Unit tests

The platform-independent unit tests are built and run on the host. Their first
configuration downloads GoogleTest.

```sh
src/tests/unit/scripts/run.sh
```

## Pinout

References:

- [Raspberry Pi Pico 2 pinout](https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf)
- [Raspberry Pi Debug Probe documentation](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html)
- [Waveshare Pico-LCD-1.3 documentation](https://www.waveshare.com/wiki/Pico-LCD-1.3)

### UART and user LED

| Pico 2 GPIO | Function |
|-------------|----------|
| GP0         | UART0 TX |
| GP1         | UART0 RX |
| GP25        | User LED |

### Waveshare Pico-LCD-1.3 display

The display uses SPI1 for clock and transmit data. D/C, chip select, and reset
are ordinary GPIO signals; the backlight uses PWM.

| Pico 2 GPIO / function | Display signal / board label |
|------------------------|------------------------------|
| GP8                    | D/C (DC)                     |
| GP9                    | Chip select (CS)             |
| GP10 (SPI1 SCK)        | SPI1 clock (CLK)             |
| GP11 (SPI1 TX)         | SPI1 MOSI (DIN)              |
| GP12                   | Reset (RST)                  |
| GP13 (PWM)             | Backlight PWM (BL)           |

### Buttons and joystick

| Pico 2 GPIO | Input              |
|-------------|--------------------|
| GP15        | Button A           |
| GP17        | Button B           |
| GP19        | Button X           |
| GP21        | Button Y           |
| GP2         | Joystick up        |
| GP18        | Joystick down      |
| GP16        | Joystick left      |
| GP20        | Joystick right     |
| GP3         | Joystick press / Z |

### AD7887 ADC

The AD8318 detector output is digitized by the external AD7887. The firmware
communicates with the ADC over SPI0.

| Pico 2 GPIO | AD7887 signal             |
|-------------|---------------------------|
| GP4         | DOUT / SPI0 MISO          |
| GP5         | Chip select               |
| GP6         | SPI0 clock                |
| GP7         | DIN / SPI0 MOSI           |
| GP14        | Acquisition timing output |

Additional hardware reference documents are stored in [`docs`](docs/).

## License

This project is licensed under the terms in [license](license).

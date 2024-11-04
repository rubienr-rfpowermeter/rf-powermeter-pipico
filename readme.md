# Pico RF-Powermeter

## Description

RF-Powermeter with RP2350 + Pico LCD 1.3 (Waveshare).

```
      RF Probe              Pi Pico               Waveshare Pico LC D1.3
   │ ┌──────────┐          ┌────────────┐        ┌──────────────────────┐
   │ │   ┌──TEMP├──────────┤AN0     SPI1├────────┼─────Display          │
   └─┼──AD318──┐│ → analog │            │ ⇆ SPI  │     240x240 1.3"     │
     │ ┌AD7787─┘│          │   RP2350   │        │                      │
     │ └────────┼──────────┤SPI0    GPIO├────────┼───┐                  │
     └──────────┘  ⇆ SPI   └────────────┘ ← dig. │   ├ Joystick 2-Axis  │
                                                 │   │ - 2-Axis + 1 PB  │
                      - RP Core 0: UI (LVGL)     │   │ - 1 PB           │
                      - RP Core 1: sampling      │   └ Keypad           │              
                                                 │     - 4 buttons      │
                                                 └──────────────────────┘
```

## Getting Started

1. Download repository
   ```bash
   cd
   git clone --recursive https://github.com/rubienr/pico-meter.git ./pico-meter
   ``` 

2. Install prerequisites
   ```bash 
   cd pico-meter/src
   scripts/install-prerequisites-ubuntu.sh
   ```

3. Set up your project to point to use the Raspberry Pi Pico SDK
    * Either by cloning the SDK locally (most common):
        1. Make sure the `pico-sdk` source is downloaded.
        2. Set `PICO_SDK_PATH` to the SDK location in your environment, or pass it (`-DPICO_SDK_PATH=`) to cmake later.

4. Build Projects
   ```bash
   scripts/cmake-make.sh
   ```

5. Upload firmware to Pico
    1. uf2 \
       Unplug Raspberry Pi Pico from Raspberry Pi and press `boot_sel` button and then connect the Raspberry Pi Pico
       back to Raspberry Pi.
       Execute following command to copy the `*.uf2` file to Pico.
       ```bash
       cp build/rf_meter.uf2 /media/pi/RPI-RP2/
       ```
    2. picoprobe
       ```bash
       scripts/make-upload-openocd.sh
       ```

* Serial monitor (picoprobe)

```bash
scripts/serial-monitor.sh
```

## Pinout

### Pico

- RP Pinout: https://datasheets.raspberrypi.com/pico/Pico-R3-A4-Pinout.pdf
- Connect Debug Probe: https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html

| Raspberry Pi Pico | Component |
|-------------------|-----------|
| GP00              | RX        |
| GP01              | TX        |
| GP25              | LED       |

### Pico LCD 1.3

https://www.waveshare.com/wiki/Pico-LCD-1.3

#### TFT

| Raspberry Pi Pico | 3.5 TFT Screen |
|-------------------|----------------|
| GP08 (SPI1)       | DC             |
| GP09 (SPI1)       | CS             |
| GP10 (SPI1)       | CLK            |
| GP11 (SPI1)       | DIN            |
| GP12 (SPI1)       | RST            |
| GP13 (SPI1)       | BL             |

#### Switches

| Raspberry Pi Pico | Component      |
|-------------------|----------------|
| GP15              | Button A       |
| GP17              | Button B       |
| GP19              | Button X       |
| GP20              | Button Y       |
| GP02              | Joystick up    |
| GP18              | Joystick down  |
| GP16              | Joystick left  |
| GP20              | Joystick right |
| GP03              | Joystick ctrl  |

### Power Detector

https://www.sv1afn.com/en/products/ad8318-digital-rf-power-detector.html

| Raspberry Pi Pico | Power Detector |
|-------------------|----------------|
| GP2 (SPI0)        | CLK            |
| GP3 (SPI0)        | DIN            |
| GP5 (SPI0)        | CS             |
| GP6 (SPI0)        | DC             |
| GP7 (SPI0)        | RST            |
| GP13(SPI0)        | BL             |








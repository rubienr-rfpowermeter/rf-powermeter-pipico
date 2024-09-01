# Pico RF-Powermeter

## Description

RF-Powermeter with "Pico Breakboard Kit" and 3.5" TFT Capacitive Touch Screen.

## Getting Started

1. Install prerequisites
   ```bash 
   sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
   ```

2. Set up your project to point to use the Raspberry Pi Pico SDK
    * Either by cloning the SDK locally (most common):
        1. Make sure the `pico-sdk` source is downloaded.
        2. Set `PICO_SDK_PATH` to the SDK location in your environment, or pass it (`-DPICO_SDK_PATH=`) to cmake later.

3. Download repository
   ```bash
   cd
   git clone --recursive https://github.com/rubienr/pico-meter.git ./pico-meter
   ```

4. Build Projects
   ```bash
   cd pico-meter
   scripts/cmake-build.sh
   ```

5. Upload firmware to Pico
    1. uf2 \
       Unplug Raspberry Pi Pico from Raspberry Pi and press `boot_sel` button and then connect the Raspberry Pi Pico
       back to Raspberry Pi.
       Execute following command to copy the `*.uf2` file to Pico.
       ```bash
       cp firmware.uf2 /media/pi/RPI-RP2/
       ```
    2. picoprobe
       ```bash
       scripts/build-upload-openocd.sh
       picicom /dev/ttyACM0 -m 115200
       ```

* Serial monitor (picoprobe)

```bash
scripts/monitor.sh
```

## TFT Features

* Resolution: 480x320 (3.5")
* Touch Type: Capacitive Touch
* TFT Controller: ST7796SU1 (SPI)
* Capacitive Touch Interface: I2C
* Input Voltage: 3.3V

## Pinout

### Components

| Raspberry Pi Pico | Component       |
|-------------------|-----------------|
| GP12 (PIO)        | RGBW LED        |
| GP13              | Buzzer          |
| GP14              | Button 2        |
| GP15              | Button 1        |
| GP16              | LED 1           |
| GP17              | LED 2           |
| 3V3               | LED 3           |
| 5V                | LED 4           |
| GP26 (ADC0)       | Joystick X-axis |                                     
| GP27 (ADC1)       | Joystick Y-axis |                                     

### TFT screen Pinout

| Raspberry Pi Pico | 3.5 TFT Screen |
|-------------------|----------------|
| GP2 (SPI0)        | CLK            |
| GP3 (SPI0)        | DIN            |
| GP5 (SPI0)        | CS             |
| GP6 (SPI0)        | DC             |
| GP7 (SPI0)        | RST            |

### Capacitive Touch Pinout

| Raspberry Pi Pico | Capacitive Touch |
|-------------------|------------------|
| GP8 (I2C0)        | SDA              |
| GP9 (I2C0)        | SCL              |

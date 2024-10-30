#include "display.h"

#include "display_config.h"
#include "display_hw_config.h"
#include "display_types.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <pico/binary_info.h>

struct PwmPeriphery
{
  uint32_t backlight_slice_nr;
  uint8_t  backlight_percent;
};

static PwmPeriphery pwm_periphery = { .backlight_percent = DISPLAY_BACKLIGHT_DEFAULT_PERCENT };
static DmaPeriphery dma_periphery = { 0 };

static void display_gpio_init()
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("SPI Display"))
  bi_decl_if_func_used(bi_4pins_with_names(
    DISPLAY_GPIO_RST, "reset, low active",
    DISPLAY_GPIO_DC, "data(1)/command(0)",
    DISPLAY_GPIO_CS, "chip select, low active",
    DISPLAY_GPIO_BL, "backlight"))
  bi_decl_if_func_used(bi_2pins_with_names(
    DISPLAY_GPIO_CLK, "SCLK",
    DISPLAY_GPIO_MOSI, "display DIN"))
    // clang-format on

    constexpr uint8_t gpios[] = { DISPLAY_GPIO_RST, DISPLAY_GPIO_DC,  DISPLAY_GPIO_CS,
                                  DISPLAY_GPIO_BL,  DISPLAY_GPIO_CLK, DISPLAY_GPIO_MOSI };

  for (auto gpio : gpios)
  {
    gpio_init(gpio);
    gpio_set_pulls(gpio, false, false);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_SLOW);
    gpio_put(gpio, false);
  }

  gpio_put(DISPLAY_GPIO_CS, true);
  gpio_put(DISPLAY_GPIO_DC, false);
}

static void display_pwm_init(PwmPeriphery &periphery)
{
  gpio_set_function(DISPLAY_GPIO_BL, GPIO_FUNC_PWM);
  periphery.backlight_slice_nr = pwm_gpio_to_slice_num(DISPLAY_GPIO_BL);
  pwm_set_wrap(periphery.backlight_slice_nr, 100);
  pwm_set_chan_level(periphery.backlight_slice_nr, PWM_CHAN_B, periphery.backlight_percent);
  pwm_set_clkdiv(periphery.backlight_slice_nr, 50);
  pwm_set_enabled(periphery.backlight_slice_nr, true);
}

static void display_spi_init()
{
#define DEBUG_DISPLAY_SPI_INIT 0
#if DEBUG_DISPLAY_SPI_INIT == 1
  const uint32_t spi_baud = { spi_init(DISPLAY_SPI_PORT, 1000 * 1000) };
#else
  const uint32_t spi_baud = { spi_init(DISPLAY_SPI_PORT, (clock_get_hz(clk_sys)) / 2 + 1) };
#endif
  spi_set_format(DISPLAY_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  printf("spi_baud=%" PRIu32 "\n", spi_baud);

  gpio_set_function(DISPLAY_GPIO_CLK, GPIO_FUNC_SPI);
  gpio_set_function(DISPLAY_GPIO_MOSI, GPIO_FUNC_SPI);
}

static void display_dma_init(DmaPeriphery &periphery)
{
  periphery.tx_dma_channel = dma_claim_unused_channel(true);
  periphery.tx_dma_config  = dma_channel_get_default_config(periphery.tx_dma_channel);
  channel_config_set_transfer_data_size(&periphery.tx_dma_config, DMA_SIZE_8);
  channel_config_set_dreq(&periphery.tx_dma_config, spi_get_dreq(DISPLAY_SPI_PORT, true));
  dma_channel_set_write_addr(periphery.tx_dma_channel, &spi_get_hw(DISPLAY_SPI_PORT)->dr, false);
  dma_channel_set_config(dma_periphery.tx_dma_channel, &dma_periphery.tx_dma_config, false);
  dma_channel_set_irq0_enabled(dma_periphery.tx_dma_channel, true);
}

static void display_set_backlight(PwmPeriphery &periphery)
{
  if (periphery.backlight_percent > 100) periphery.backlight_percent = 100;
  else if (periphery.backlight_percent < DISPLAY_BACKLIGHT_MIN_PERCENT)
    periphery.backlight_percent = DISPLAY_BACKLIGHT_MIN_PERCENT;
  pwm_set_chan_level(periphery.backlight_slice_nr, PWM_CHAN_B, periphery.backlight_percent);
}

static void display_reset()
{
  // see ST7789VW V1.0, page 48, section 7.4.5 Reset Timing
  gpio_put(DISPLAY_GPIO_RST, true);
  sleep_ms(100);
  gpio_put(DISPLAY_GPIO_RST, false);
  sleep_ms(100);   // T_RW min 10µs
  gpio_put(DISPLAY_GPIO_RST, true);
  sleep_ms(120);   // T_RT max 120ms
}

static void display_send_command(uint8_t command)
{
  gpio_put(DISPLAY_GPIO_DC, false);
  gpio_put(DISPLAY_GPIO_CS, false);
  spi_write_blocking(DISPLAY_SPI_PORT, &command, sizeof(command));
  gpio_put(DISPLAY_GPIO_CS, true);
}

static void display_send_data_1byte(uint8_t data)
{
  gpio_put(DISPLAY_GPIO_DC, true);
  gpio_put(DISPLAY_GPIO_CS, false);
  spi_write_blocking(DISPLAY_SPI_PORT, &data, 1);
  gpio_put(DISPLAY_GPIO_CS, true);
}

static void display_send_data_nbyte(const uint8_t *data, uint8_t length)
{
  gpio_put(DISPLAY_GPIO_DC, true);
  gpio_put(DISPLAY_GPIO_CS, false);
  spi_write_blocking(DISPLAY_SPI_PORT, data, length);
  gpio_put(DISPLAY_GPIO_CS, true);
}

static void display_set_scanning_method(enum DisplayScanDirection scan_direction)
{
  // see st7789vw, V1.0, page 125
  uint8_t           my   = { 0 };   // d7, page address order, 0 top to bottom, 1 bottom to top
  uint8_t           mx   = { 0 };   // d6, column address order, 0 left to right, 1 right to left
  uint8_t           mv   = { 1 };   // d5, page/column order, 0 normal, 1 reverse
  uint8_t           ml   = { 1 };   // d4, line address order, 0 refresh top to bottom, 1 refresh bottom to top
  constexpr uint8_t rgb  = { 0 };   // d3, rgb/bgr order, 0 rgb, 1 bgr
  uint8_t           mh   = { 0 };   // d2, display data latch data order, 0 refresh left to right, 1 refresh right to left
  constexpr uint8_t rfu2 = { 1 };   // d1
  constexpr uint8_t rfu1 = { 0 };   // d0

  // get GRAM and display width and height
  if (DisplayScanDirection_0_DEG == scan_direction)
  {
    my = 0;
    mx = 0;
    mv = 0;
    ml = 0;
    mh = 0;
  }
  else   // ScanDirection_90_DEG
  {
    my = 0;
    mx = 1;
    mv = 1;
    ml = 1;
    mh = 0;
  }

  display_send_command(0x36);   // MADCTL, memory data access control, st7789vw, V1.0, page 214, section 9.1.28
  display_send_data_1byte(my << 7 | mx << 6 | mv << 5 | ml << 4 | rgb << 3 | mh << 2 | rfu2 << 1 | rfu1 << 0);
}

static void display_init_registers()
{

  display_send_command(0x3a);   // COLMOD, interface pixel format, st7789vw V1.0, page 224, section 9.1.32
  display_send_data_1byte(
    0 << 7 |                     // d7, set to zero
    1 << 6 | 0 << 5 | 1 << 4 |   // d[6:4], 0b101: 65k RGB
    0 << 3 |                     // d3, set to zero
    1 << 2 | 0 << 1 | 1 << 0);   // d[2:0], 0b101: 16bit/pixel

  // PORCTRL, porch setting, st7789vw V1.0, page 263, section 9.2.3
  display_send_command(0xb2);
  display_send_data_1byte(0x0c);     // back porch in normal mode
  display_send_data_1byte(0x0c);     // front porch in normal mode
  display_send_data_1byte(0 << 0);   // d0, PSEN, 0: disable separate porch control, 1: enable separate porch control
  display_send_data_1byte(0x33);     // front and back porch in idle mode
  display_send_data_1byte(0x33);     // front and back porch in partial mode

  // GCTRL, gate control, st7789vw V1.0, page 267, section 9.2.6
  display_send_command(0xb7);
  display_send_data_1byte(
    0 << 7 |                     // d7, set to zero
    0 << 6 | 1 << 5 | 1 << 4 |   // VGHS[2:0]
    0 << 3 |                     // d3, set to zero
    1 << 2 | 0 << 1 | 1 << 0);   // VGHL[2:0]

  // VCOMS, VCOM setting, st7789vw V1.0, page 272, section 9.2.9
  display_send_command(0xbb);
  display_send_data_1byte(0x19);   // VCOMS[5:0], 0x19: 0.725V

  // LCMCTRL, LCM Control, st7789vw V1.0, page 276, section 9.2.12
  display_send_command(0xc0);
  display_send_data_1byte(
    0 << 7 |   // d7, set to zero
    0 << 6 |   // d6, XMY, XOR MY setting in 0x36
    1 << 5 |   // d5, XBGR, xor rgb setting in 0x36
    0 << 4 |   // d4, XIV, xor inverse setting in command 0x21
    1 << 3 |   // d3, XMX, xor mx setting in command 0x36
    1 << 2 |   // d2, XMH can reverse source output order (only for RGB without RAM)
    0 << 1 |   // d1, XMV, xor mv setting in command 0x36
    0 << 0);   // d0, XGS xor gs setting in Command 0x34

  // VDVVRHEN, VDV and VRH command enable, st7789vw V1.0, page 278, section 9.2.14
  display_send_command(0xC2);
  display_send_data_1byte(1 << 0);   // d0, CMDEN,
                                     // 0: VDV and VRH register value comes from NVM,
                                     // 1: VDV and VRH register value comes from command write

  // VRHS, VRH set, st7789vw V1.0, st7789vw V1.0, page 279, section 9.2.15
  display_send_command(0xC3);
  display_send_data_1byte(0x12);   // VRHS[5:0], 0x12:
                                   // VAP=(4.45V + (vcom + vcom_offset + vdv))
                                   // VAN=(-4.45V + (vcom + vcom_offset - vdv))

  // VDVS, VRH set, st7789vw V1.0, page 281, section 9.2.16
  display_send_command(0xc4);
  display_send_data_1byte(0x20);   // VDVS[5:0] 0x20: VDV=0V

  // FRCTRL2, frame rate control in normal mode, st7789vw V1.0, page 285, section 9.2.18
  display_send_command(0xc6);
  display_send_data_1byte(
    0 << 7 | 0 << 6 | 0 << 5 |                     // NLA[2:0], 0x00 dot inversion, 0x05 column inversion
    0 << 4 | 1 << 3 | 1 << 2 | 0 << 1 | 0 << 0);   // RTNA[4:0], 0x0f: 60f/s in normal mode

  // PWCTRL1 (D0h): Power Control 1, st7789vw V1.0, page 291, section 9.2.23
  display_send_command(0xd0);
  display_send_data_1byte(0b10100100);   // constant
  display_send_data_1byte(
    1 << 7 | 0 << 6 |   // AVDD[1:0] 0x02: AVDD =6.8V
    1 << 5 | 0 << 4 |   // AVCL[1:0] 0x02: AVCL=-4.8V
    0 << 3 | 0 << 2 |   // d[3:2] set to zero
    0 << 1 | 1 << 0);   // VDS[1:0] 0x01: VDDS=2.3V

  // PVGAMCTRL, positive voltage gamma control, st7789vw V1.0, page 295, section 9.2.26
  display_send_command(0xe0);
  display_send_data_1byte(0xD0);
  display_send_data_1byte(0x04);
  display_send_data_1byte(0x0D);
  display_send_data_1byte(0x11);
  display_send_data_1byte(0x13);
  display_send_data_1byte(0x2B);
  display_send_data_1byte(0x3F);
  display_send_data_1byte(0x54);
  display_send_data_1byte(0x4C);
  display_send_data_1byte(0x18);
  display_send_data_1byte(0x0D);
  display_send_data_1byte(0x0B);
  display_send_data_1byte(0x1F);
  display_send_data_1byte(0x23);

  // NVGAMCTRL, negative voltage gamma control, st7789vw V1.0, page 297, section 9.2.24
  display_send_command(0xe1);
  display_send_data_1byte(0xD0);
  display_send_data_1byte(0x04);
  display_send_data_1byte(0x0C);
  display_send_data_1byte(0x11);
  display_send_data_1byte(0x13);
  display_send_data_1byte(0x2C);
  display_send_data_1byte(0x3F);
  display_send_data_1byte(0x44);
  display_send_data_1byte(0x51);
  display_send_data_1byte(0x2F);
  display_send_data_1byte(0x1F);
  display_send_data_1byte(0x1F);
  display_send_data_1byte(0x20);
  display_send_data_1byte(0x23);

  // INVON, display inversion on, st7789vw V1.0, page 190, section 9.1.16
  display_send_command(0x21);

  // SLPOUT, sleep out (turn sleep off), st7789vw V1.0, page 184, section 9.1.12
  display_send_command(0x11);

  // DISPON, display on (recover from display off), st7789vw V1.0, page 196, section 9.1.19
  display_send_command(0x29);
}

void display_set_window(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
  const uint8_t x_data[] = { (uint8_t)((start_x & 0xff00) >> 8),       // XS[15:8]
                             (uint8_t)(start_x & 0x00ff),              // XS[7:0]
                             (uint8_t)(((end_x - 1) & 0xff00) >> 8),   // XE[15:8]
                             (uint8_t)((end_x - 1) & 0x00ff) };        // XE[7:0]

  const uint8_t y_data[] = { (uint8_t)((start_y & 0xff00) >> 8),       // YS[15:8]
                             (uint8_t)(start_y & 0x00ff),              // YS[7:0]
                             (uint8_t)(((end_y - 1) & 0xff00) >> 8),   // YE[15:8]
                             (uint8_t)((end_y - 1) & 0x00ff) };        // YE[7:0]

  // CASET, column address set (column line in frame memory), st7789vw V1.0, page 198, section 9.1.20
  display_send_command(0x2a);
  display_send_data_nbyte(x_data, sizeof(x_data));

  // RASET, row address set (page line in frame memory), st7789vw V1.0, page 200, section 9.1.21
  display_send_command(0x2b);
  display_send_data_nbyte(y_data, sizeof(y_data));

  // RAMWR, memory write, st7789vw V1.0, page 202, section 9.1.22
  display_send_command(0x2c);
}

void display_clear(enum DisplayColor color)
{
  display_set_window(0, 0, DISPLAY_WIDTH_PX, DISPLAY_HEIGHT_PX);

  uint16_t       line[DISPLAY_WIDTH_PX];
  const uint16_t clr = { (uint16_t)(((color & 0x00ff) << 8) | ((color & 0xff00) >> 8)) };

  for (auto &i : line)
    i = clr;

  gpio_put(DISPLAY_GPIO_DC, true);
  gpio_put(DISPLAY_GPIO_CS, false);
  for (size_t i = 0; i < DISPLAY_HEIGHT_PX; i++)
    spi_write_blocking(DISPLAY_SPI_PORT, (uint8_t *)&line, sizeof(line));

  gpio_put(DISPLAY_GPIO_CS, true);
}

void display_init()
{
  display_gpio_init();
  display_pwm_init(pwm_periphery);
  display_spi_init();
  display_dma_init(dma_periphery);

  display_set_backlight(pwm_periphery);
  display_reset();
  display_set_scanning_method(DISPLAY_DIRECTION);
  display_init_registers();
  display_clear(DisplayColor_GREEN);
}

DmaPeriphery &display_get_dma_periphery() { return dma_periphery; }

uint32_t display_get_baud_rate() { return spi_get_baudrate(DISPLAY_SPI_PORT); }

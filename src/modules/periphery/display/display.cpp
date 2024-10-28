#include "display.h"

#include "display_config.h"
#include "display_hw_config.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include <cinttypes>
#include <cstdio>

typedef struct
{
  uint32_t backlight_slice_nr;
  uint8_t backlight_percent;
} PwmPeriphery;

static PwmPeriphery pwm_periphery = { .backlight_percent = LCD_BACKLIGHT_DEFAULT_PERCENT };
static DmaPeriphery dma_periphery = { 0 };


static void lcd_gpio_init()
{
  constexpr uint8_t gpios[] = { LCD_GPIO_RST, LCD_GPIO_DC, LCD_GPIO_CS, LCD_GPIO_BL, LCD_GPIO_CLK, LCD_GPIO_MOSI };

  for(auto gpio : gpios)
  {
    gpio_init(gpio);
    gpio_set_pulls(gpio, false, false);
    gpio_set_dir(gpio, GPIO_OUT);
    // gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_2MA);
    // gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_SLOW);
    gpio_put(gpio, false);
  }

  gpio_put(LCD_GPIO_CS, true);
  gpio_put(LCD_GPIO_DC, false);
}

static void lcd_pwm_init(PwmPeriphery &periphery)
{
  gpio_set_function(LCD_GPIO_BL, GPIO_FUNC_PWM);
  periphery.backlight_slice_nr = pwm_gpio_to_slice_num(LCD_GPIO_BL);
  pwm_set_wrap(periphery.backlight_slice_nr, 100);
  pwm_set_chan_level(periphery.backlight_slice_nr, PWM_CHAN_B, periphery.backlight_percent);
  pwm_set_clkdiv(periphery.backlight_slice_nr, 50);
  pwm_set_enabled(periphery.backlight_slice_nr, true);
}

static void lcd_spi_init()
{
#define DEBUIG_LCD_SPI_INIT 0
#if DEBUIG_LCD_SPI_INIT == 1
  const uint32_t spi_baud = { spi_init(LCD_SPI_PORT, 1000 * 1000) };
#else
  const uint32_t spi_baud = { spi_init(LCD_SPI_PORT, 62.5 * 1000 * 1000) };
#endif
  spi_set_format(LCD_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  printf("spi_baud=%" PRIu32 "\n", spi_baud);

  gpio_set_function(LCD_GPIO_CLK, GPIO_FUNC_SPI);
  gpio_set_function(LCD_GPIO_MOSI, GPIO_FUNC_SPI);
}

static void lcd_dma_init(DmaPeriphery &periphery)
{
  periphery.tx_dma_channel = dma_claim_unused_channel(true);
  periphery.tx_dma_config  = dma_channel_get_default_config(periphery.tx_dma_channel);
  channel_config_set_transfer_data_size(&periphery.tx_dma_config, DMA_SIZE_8);
  channel_config_set_dreq(&periphery.tx_dma_config, spi_get_dreq(LCD_SPI_PORT, true));
}

static void lcd_set_backlight(PwmPeriphery &periphery)
{
  if(periphery.backlight_percent > 100) periphery.backlight_percent = 100;
  else if(periphery.backlight_percent < LCD_BACKLIGHT_MIN_PERCENT) periphery.backlight_percent = LCD_BACKLIGHT_MIN_PERCENT;
  pwm_set_chan_level(periphery.backlight_slice_nr, PWM_CHAN_B, periphery.backlight_percent);
}

static void lcd_reset()
{
  // see ST7789VW V1.0, page 48, section 7.4.5 Reset Timing
  gpio_put(LCD_GPIO_RST, true);
  sleep_ms(100);
  gpio_put(LCD_GPIO_RST, false);
  sleep_ms(100); // T_RW min 10µs
  gpio_put(LCD_GPIO_RST, true);
  sleep_ms(120); // T_RT max 120ms
}

static void lcd_send_command(uint8_t command)
{
  gpio_put(LCD_GPIO_DC, false);
  gpio_put(LCD_GPIO_CS, false);
  spi_write_blocking(LCD_SPI_PORT, &command, sizeof(command));
  gpio_put(LCD_GPIO_CS, true);
}

static void lcd_send_data_1byte(uint8_t data)
{
  gpio_put(LCD_GPIO_DC, true);
  gpio_put(LCD_GPIO_CS, false);
  spi_write_blocking(LCD_SPI_PORT, &data, sizeof(data));
  gpio_put(LCD_GPIO_CS, true);
}

static void lcd_set_scanning_method(enum LcdScanDirection scan_direction)
{
  // see st7789vw, V1.0, page 125
  uint8_t my             = { 0 }; // d7, page address order, 0 top to bottom, 1 bottom to top
  uint8_t mx             = { 0 }; // d6, column address order, 0 left to right, 1 right to left
  uint8_t mv             = { 1 }; // d5, page/column order, 0 normal, 1 reverse
  uint8_t ml             = { 1 }; // d4, line address order, 0 refresh top to bottom, 1 refresh bottom to top
  constexpr uint8_t rgb  = { 0 }; // d3, rgb/bgr order, 0 rgb, 1 bgr
  constexpr uint8_t mh   = { 0 }; // d2, display data latch data order, 0 refresh left to right, 1 refresh right to left
  constexpr uint8_t rfu2 = { 1 }; // d1
  constexpr uint8_t rfu1 = { 0 }; // d0

  // Get GRAM and LCD width and height
  if(LcdScanDirection_0_DEG == scan_direction)
  {
    mx = 0; // d6, column address order, 0: left to right, 1: right to left
    mv = 0; // d5, page/column order, 0: normal, 1: reverse
    ml = 0; // d4, line address order, 0: refresh top to bottom, 1: refresh bottom to top
  }
  else // ScanDirection_90_DEG
  {
    mx = 1; // d6, column address order, 0: left to right, 1: right to left
    mv = 1; // d5, page/column order, 0: normal, 1: reverse
    ml = 1; // d4, line address order, 0: refresh top to bottom, 1: refresh bottom to top
  }

  lcd_send_command(0x36); // MADCTL, memory data access control, st7789vw, V1.0, page 214, section 9.1.28
  lcd_send_data_1byte(my << 7 | mx << 6 | mv << 5 | ml << 4 | rgb << 3 | mh << 2 | rfu2 << 1 | rfu1 << 0);
}

static void lcd_init_registers()
{

  lcd_send_command(0x3a); // COLMOD, interface pixel format, st7789vw V1.0, page 224, section 9.1.32
  lcd_send_data_1byte(
    0 << 7                       // d7, set to zero
    | 1 << 6 | 0 << 5 | 1 << 4   // d[6:4], 0b101: 65k RGB
    | 0 << 3                     // d3, set to zero
    | 1 << 2 | 0 << 1 | 1 << 0); // d[2:0], 0b101: 16bit/pixel

  // PORCTRL, porch setting, st7789vw V1.0, page 263, section 9.2.3
  lcd_send_command(0xb2);
  lcd_send_data_1byte(0x0c);   // back porch in normal mode
  lcd_send_data_1byte(0x0c);   // front porch in normal mode
  lcd_send_data_1byte(0 << 0); // d0, PSEN, 0: disable separate porch control, 1: enable separate porch control
  lcd_send_data_1byte(0x33);   // front and back porch in idle mode
  lcd_send_data_1byte(0x33);   // front and back porch in partial mode

  // GCTRL, gate control, st7789vw V1.0, page 267, section 9.2.6
  lcd_send_command(0xb7);
  lcd_send_data_1byte(
    0 << 7                       // d7, set to zero
    | 0 << 6 | 1 << 5 | 1 << 4   // VGHS[2:0]
    | 0 << 3                     // d3, set to zero
    | 1 << 2 | 0 << 1 | 1 << 0); // VGHL[2:0]

  // VCOMS, VCOM setting, st7789vw V1.0, page 272, section 9.2.9
  lcd_send_command(0xbb);
  lcd_send_data_1byte(0x19); // VCOMS[5:0], 0x19: 0.725V

  // LCMCTRL, LCM Control, st7789vw V1.0, page 276, section 9.2.12
  lcd_send_command(0xc0);
  lcd_send_data_1byte(
    0 << 7     // d7, set to zero
    | 0 << 6   // d6, XMY, XOR MY setting in 0x36
    | 1 << 5   // d5, XBGR, xor rgb setting in 0x36
    | 0 << 4   // d4, XIV, xor inverse setting in command 0x21
    | 1 << 3   // d3, XMX, xor mx setting in command 0x36
    | 1 << 2   // d2, XMH can reverse source output order (only for RGB without RAM)
    | 0 << 1   // d1, XMV, xor mv setting in command 0x36
    | 0 << 0); // d0, XGS xor gs setting in Command 0x34

  // VDVVRHEN, VDV and VRH command enable, st7789vw V1.0, page 278, section 9.2.14
  lcd_send_command(0xC2);
  lcd_send_data_1byte(1 << 0); // d0, CMDEN,
                               // 0: VDV and VRH register value comes from NVM,
                               // 1: VDV and VRH register value comes from command write

  // VRHS, VRH set, st7789vw V1.0, st7789vw V1.0, page 279, section 9.2.15
  lcd_send_command(0xC3);
  lcd_send_data_1byte(0x12); // VRHS[5:0], 0x12:
                             // VAP=(4.45V + (vcom + vcom_offset + vdv))
                             // VAN=(-4.45V + (vcom + vcom_offset - vdv))

  // VDVS, VRH set, st7789vw V1.0, page 281, section 9.2.16
  lcd_send_command(0xc4);
  lcd_send_data_1byte(0x20); // VDVS[5:0] 0x20: VDV=0V

  // FRCTRL2, frame rate control in normal mode, st7789vw V1.0, page 285, section 9.2.18
  lcd_send_command(0xc6);
  lcd_send_data_1byte(
    0 << 7 | 0 << 6 | 0 << 5                       // NLA[2:0], 0x00 dot inversion, 0x05 column inversion
    | 0 << 4 | 1 << 3 | 1 << 2 | 0 << 1 | 0 << 0); // RTNA[4:0], 0x0f: 60f/s in normal mode

  // PWCTRL1 (D0h): Power Control 1, st7789vw V1.0, page 291, section 9.2.23
  lcd_send_command(0xd0);
  lcd_send_data_1byte(0b10100100); // constant
  lcd_send_data_1byte(
    1 << 7 | 0 << 6     // AVDD[1:0] 0x02: AVDD =6.8V
    | 1 << 5 | 0 << 4   // AVCL[1:0] 0x02: AVCL=-4.8V
    | 0 << 3 | 0 << 2   // d[3:2] set to zero
    | 0 << 1 | 1 << 0); // VDS[1:0] 0x01: VDDS=2.3V

  // PVGAMCTRL, positive voltage gamma control, st7789vw V1.0, page 295, section 9.2.26
  lcd_send_command(0xe0);
  lcd_send_data_1byte(0xD0);
  lcd_send_data_1byte(0x04);
  lcd_send_data_1byte(0x0D);
  lcd_send_data_1byte(0x11);
  lcd_send_data_1byte(0x13);
  lcd_send_data_1byte(0x2B);
  lcd_send_data_1byte(0x3F);
  lcd_send_data_1byte(0x54);
  lcd_send_data_1byte(0x4C);
  lcd_send_data_1byte(0x18);
  lcd_send_data_1byte(0x0D);
  lcd_send_data_1byte(0x0B);
  lcd_send_data_1byte(0x1F);
  lcd_send_data_1byte(0x23);

  // NVGAMCTRL, negative voltage gamma control, st7789vw V1.0, page 297, section 9.2.24
  lcd_send_command(0xe1);
  lcd_send_data_1byte(0xD0);
  lcd_send_data_1byte(0x04);
  lcd_send_data_1byte(0x0C);
  lcd_send_data_1byte(0x11);
  lcd_send_data_1byte(0x13);
  lcd_send_data_1byte(0x2C);
  lcd_send_data_1byte(0x3F);
  lcd_send_data_1byte(0x44);
  lcd_send_data_1byte(0x51);
  lcd_send_data_1byte(0x2F);
  lcd_send_data_1byte(0x1F);
  lcd_send_data_1byte(0x1F);
  lcd_send_data_1byte(0x20);
  lcd_send_data_1byte(0x23);

  // INVON, display inversion on, st7789vw V1.0, page 190, section 9.1.16
  lcd_send_command(0x21);

  // SLPOUT, sleep out (turn sleep off), st7789vw V1.0, page 184, section 9.1.12
  lcd_send_command(0x11);

  // DISPON, display on (recover from display off), st7789vw V1.0, page 196, section 9.1.19
  lcd_send_command(0x29);
}


void lcd_set_window(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
  // CASET, column address set (column line in frame memory), st7789vw V1.0, page 198, section 9.1.20
  lcd_send_command(0x2a);
  lcd_send_data_1byte(0x00);      // XS[15:8]
  lcd_send_data_1byte(start_x);   // XS[7:0]
  lcd_send_data_1byte(0x00);      // XE[15:8]
  lcd_send_data_1byte(end_x - 1); // XE[7:0]

  // RASET, row address set (page line in frame memory), st7789vw V1.0, page 200, section 9.1.21
  lcd_send_command(0x2b);
  lcd_send_data_1byte(0x00);      // YS[15:8]
  lcd_send_data_1byte(start_y);   // YS[7:0]
  lcd_send_data_1byte(0x00);      // YE[15:8]
  lcd_send_data_1byte(end_y - 1); // YE[7:0]

  // RAMWR, memory write, st7789vw V1.0, page 202, section 9.1.22
  lcd_send_command(0x2c);
}

void lcd_clear(enum LcdColor color)
{
  lcd_set_window(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX);

  uint16_t line[LCD_WIDTH_PX];
  const uint16_t clr = { (uint16_t)(((color & 0x00ff) << 8) | ((color & 0xff00) >> 8)) };

  for(auto &i : line)
    i = clr;

  gpio_put(LCD_GPIO_DC, true);
  gpio_put(LCD_GPIO_CS, false);
  for(size_t i = 0; i < LCD_HEIGHT_PX; i++)
    spi_write_blocking(LCD_SPI_PORT, (uint8_t *)&line, sizeof(line));

  gpio_put(LCD_GPIO_CS, true);
}

void display_init()
{
  lcd_gpio_init();
  lcd_pwm_init(pwm_periphery);
  lcd_spi_init();
  lcd_dma_init(dma_periphery);

  lcd_set_backlight(pwm_periphery);
  lcd_reset();
  lcd_set_scanning_method(LCD_DIRECTION);
  lcd_init_registers();
  lcd_clear(LcdColor_GREEN);
}

DmaPeriphery &display_get_dma_periphery() { return dma_periphery; }

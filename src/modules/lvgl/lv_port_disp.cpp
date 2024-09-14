#include "lv_port_disp.h"
#include <pico/binary_info/code.h>
#include <pico/printf.h>

#include "hardware/spi.h"
#include "lib/types/cast.h"
#include "modules/globals/globals.h"
#include "pico/stdlib.h"

#include <hardware/dma.h>

typedef struct
{
  uint8_t cmd;
  uint8_t data[16];
  uint8_t databytes; //! Not part of in-data; bit 7 = delay after set; 0xFF = end of commands.
} lcd_init_cmd_t;

static const int gpio_spi_sck = 2;
static const int gpio_spi_tx  = 3;
static const int gpio_spi_csn = 5;
static const int gpio_dc      = 6;
static const int gpio_rst     = 7;

constexpr uint16_t display_horizontal_px = { 480 };
constexpr uint16_t display_vertical_px   = { 320 };
constexpr uint16_t dizplay_buffer_items  = { display_horizontal_px * 40 };

//! 0x48: PORTRAIT, 0x88: PORTRAIT_INVERTED, 0x28: LANDSCAPE, 0xE8: LANDSCAPE_INVERTED
constexpr uint8_t disp_orientation = { 0x28 };

static volatile bool disp_flush_enabled = true;
static spi_inst_t *display_spi          = { spi0 };
static int dma_tx;
static volatile lv_disp_drv_t *call_when_dma_finished = { nullptr };

static void st7796s_send_cmd(uint8_t cmd)
{
  gpio_put(19, true); // todo rr - to be removed

  if(dma_channel_is_busy(dma_tx)) { printf("lv_port_disp: warning dma_channel=%u busy\n", dma_tx); }
  while(dma_channel_is_busy(dma_tx)) { }

  gpio_put(gpio_dc, false);
  gpio_put(gpio_spi_csn, false);
  sleep_us(1);

  dma_channel_set_read_addr(dma_tx, &cmd, false);
  dma_channel_set_trans_count(dma_tx, 1, true);

  // CSn is released by DMA ISR0 handler

  gpio_put(19, false); // todo rr - to be removed
}

static void st7796s_send_data(void *data, uint16_t length)
{
  gpio_put(18, true); // todo rr - to be removed

  if(dma_channel_is_busy(dma_tx)) { printf("lv_port_disp: warning dma_channel=%u busy\n", dma_tx); }
  while(dma_channel_is_busy(dma_tx)) { }

  gpio_put(gpio_dc, true);
  gpio_put(gpio_spi_csn, false);
  sleep_us(1);

  dma_channel_set_read_addr(dma_tx, data, false);
  dma_channel_set_trans_count(dma_tx, length, true);

  // CSn is released by DMA ISR0 handler

  gpio_put(18, false); // todo rr - to be removed
}

static void st7796s_send_color(void *data, size_t length)
{
  if(dma_channel_is_busy(dma_tx)) { printf("lv_port_disp: warning dma_channel=%u busy\n", dma_tx); }
  while(dma_channel_is_busy(dma_tx)) { }

  gpio_put(gpio_dc, true);
  gpio_put(gpio_spi_csn, false);
  sleep_us(1);

  dma_channel_set_read_addr(dma_tx, data, false);
  dma_channel_set_trans_count(dma_tx, length, true);

  // CSn is released by DMA ISR0 handler
}

//! Flush the content of the internal buffer the specific area on the display
//! You can use DMA or any hardware acceleration to do this operation in the background but
//! 'lv_disp_flush_ready()' has to be called when finished.
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
  if(disp_flush_enabled)
  {
    uint8_t data[4];

    // column addresses
    st7796s_send_cmd(0x2A);
    data[0] = (area->x1 >> 8) & 0xFF;
    data[1] = area->x1 & 0xFF;
    data[2] = (area->x2 >> 8) & 0xFF;
    data[3] = area->x2 & 0xFF;
    st7796s_send_data(data, 4);

    // page addresses
    st7796s_send_cmd(0x2B);
    data[0] = (area->y1 >> 8) & 0xFF;
    data[1] = area->y1 & 0xFF;
    data[2] = (area->y2 >> 8) & 0xFF;
    data[3] = area->y2 & 0xFF;
    st7796s_send_data(data, 4);

    // memory write
    st7796s_send_cmd(0x2C);

    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
    st7796s_send_color((void *)color_p, size * 2);

    // todo rr - clarify if:
    //  - the call is expected in this context or can be in ISR context
    while(call_when_dma_finished != nullptr) { }
    call_when_dma_finished = disp_drv;
    // lv_disp_flush_ready(disp_drv);
  }
}

void dma_irq0_handler()
{
  dma_hw->ints0 = 1u << dma_tx;
  busy_wait_us(1);
  gpio_put(gpio_spi_csn, true);

  if(nullptr != call_when_dma_finished)
  {
    lv_disp_flush_ready((lv_disp_drv_t *)call_when_dma_finished);
    call_when_dma_finished = nullptr;
  }
}

static void st7796s_set_orientation(uint8_t orientation)
{
  st7796s_send_cmd(0x36);

  gpio_put(gpio_dc, true);
  gpio_put(gpio_spi_csn, false);
  sleep_us(1);

  spi_write_blocking(display_spi, (uint8_t *){ &orientation }, 1);

  sleep_us(1);
  gpio_put(gpio_spi_csn, true);
}

//! Initialize your display and the required peripherals.
static void disp_init()
{
  lcd_init_cmd_t init_cmds[] = {
    // {0xCF, {0x00, 0x83, 0X30}, 3},    // ?
    { 0xED,                                                                   { 0x64, 0x03, 0X12, 0X81 },    4 }, // DOCA: display output ctrl adjust
    { 0xE8,                                                                         { 0x85, 0x01, 0x79 },    3 }, // DOCA: display output ctrl adjust
    // {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5}, // ?
    // {0xF7, {0x20}, 1},                // ?
    { 0xEA,                                                                               { 0x00, 0x00 },    2 }, // DOCA: display output ctrl adjust
    { 0xC0,                                                                                     { 0x26 },    1 }, // PWR1: power control 1
    { 0xC1,                                                                                     { 0x11 },    1 }, // PWR1: power control 1
    { 0xC5,                                                                               { 0x35, 0x3E },    2 }, // VCMPCTL: vcom control
    { 0xC7,                                                                                     { 0xBE },    1 }, // VCM Offset: vcom offset register
    { 0x36,                                                                                     { 0x28 },    1 }, // MADCTL: memory data access control
    { 0x3A,                                                                                     { 0x05 },    1 }, // COLMOD: Interface pixel format
    // {0xB1, {0x00, 0x1B}, 2},          // FRMCTR1: frame rate control
    { 0xB1,                                                                               { 0x00, 0x00 },    2 }, // FRMCTR1: frame rate control
    // {0xF2, {0x08}, 1},                // ?
    // {0x26, {0x01}, 1},                // ?
    { 0xE0, { 0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00 },   15 }, // PGC: positive  gamma control
    { 0XE1, { 0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F },   15 }, // NGC: negative  gamma control
    { 0x2A,                                                                   { 0x00, 0x00, 0x00, 0xEF },    4 }, // CASET: column address set
    { 0x2B,                                                                   { 0x00, 0x00, 0x01, 0x3f },    4 }, // RASET: row address set
    { 0x2C,                                                                                        { 0 },    0 }, // RAMWR: memory write
    { 0xB7,                                                                                     { 0x07 },    1 }, // EM: entry mode wet
    { 0xB6,                                                                   { 0x0A, 0x82, 0x27, 0x00 },    4 }, // DFC: display function control
    { 0x11,                                                                                        { 0 }, 0x80 }, // SLP: sleep out
    { 0x29,                                                                                        { 0 }, 0x80 }, // DISPON: display on
    {    0,                                                                                        { 0 }, 0xff }, // NOP: no operation
  };

  // clang-format off
  bi_decl_if_func_used(bi_program_feature("TFT 480x320 (SPI0)"))
  bi_decl_if_func_used(bi_3pins_with_func(gpio_spi_sck, gpio_spi_tx, gpio_spi_csn, GPIO_FUNC_SPI))
  bi_decl_if_func_used(bi_4pins_with_names(gpio_spi_sck, "TFT (SCLK)", gpio_spi_tx, "TFT (MOSI)", 4, "TFT (MISO)", gpio_spi_csn, "TFT (CS)"))
  bi_decl_if_func_used(bi_2pins_with_names(gpio_dc, "TFT (DC)", gpio_rst, "TFT (RST)"))
    // clang-format on

    constexpr uint target_baud_rate = { 62500000 };
  const uint effective_baud_rate    = { spi_init(spi0, target_baud_rate) };
  printf("lv_port_disp: spi0 effective_clock_hz=%u requested_clock_hz=%u\n", effective_baud_rate, target_baud_rate);
  globals.display.baud_rate = effective_baud_rate,

  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  gpio_init(gpio_spi_tx);
  gpio_init(gpio_spi_sck);
  gpio_init(gpio_spi_csn);
  gpio_init(gpio_dc);
  gpio_init(gpio_rst);

  gpio_set_function(gpio_spi_tx, GPIO_FUNC_SPI);
  gpio_set_function(gpio_spi_sck, GPIO_FUNC_SPI);

  gpio_set_drive_strength(gpio_spi_csn, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_drive_strength(gpio_dc, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_drive_strength(gpio_rst, GPIO_DRIVE_STRENGTH_2MA);

  gpio_set_dir(gpio_spi_csn, GPIO_OUT);
  gpio_set_dir(gpio_dc, GPIO_OUT);
  gpio_set_dir(gpio_rst, GPIO_OUT);

  gpio_put(gpio_spi_csn, true);
  gpio_put(gpio_dc, true);

  gpio_put(gpio_rst, true);
  sleep_ms(100);
  gpio_put(gpio_rst, false);
  sleep_ms(100);
  gpio_put(gpio_rst, true);
  sleep_ms(100);

  // Send all the commands
  uint16_t cmd = 0;
  while(init_cmds[cmd].databytes != 0xff)
  {
    st7796s_send_cmd(init_cmds[cmd].cmd);
    st7796s_send_data(init_cmds[cmd].data, init_cmds[cmd].databytes & 0x1F);
    if(init_cmds[cmd].databytes & 0x80) { sleep_ms(100); }
    cmd++;
  }

  st7796s_set_orientation(disp_orientation);

  st7796s_send_cmd(0x21);

  // todo rr - to be removed
  gpio_init(18);
  gpio_put(18, false);
  gpio_disable_pulls(18);
  gpio_set_drive_strength(18, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_dir(18, GPIO_OUT);

  gpio_init(19);
  gpio_put(19, false);
  gpio_disable_pulls(19);
  gpio_set_drive_strength(19, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_dir(19, GPIO_OUT);
}

void lv_port_disp_init(void)
{
  dma_tx                           = dma_claim_unused_channel(true);
  dma_channel_config dma_tx_config = dma_channel_get_default_config(dma_tx);

  channel_config_set_transfer_data_size(&dma_tx_config, DMA_SIZE_8);
  channel_config_set_dreq(&dma_tx_config, spi_get_dreq(display_spi, true));
  channel_config_set_read_increment(&dma_tx_config, true);
  dma_channel_configure(
    dma_tx, &dma_tx_config,
    &spi_get_hw(display_spi)->dr, // write address
    nullptr,                      // read address
    0,                            // length
    false);                       // don't start

  // triggers clearing of CSn-line after DMA has finished writing its block
  dma_channel_set_irq0_enabled(dma_tx, true);
  irq_set_exclusive_handler(DMA_IRQ_0, dma_irq0_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  disp_init();

  static lv_disp_draw_buf_t draw_buf_dsc_1;
  static lv_color_t buffer_1[dizplay_buffer_items];
  static lv_color_t buffer_2[dizplay_buffer_items];
  lv_disp_draw_buf_init(&draw_buf_dsc_1, buffer_1, buffer_2, dizplay_buffer_items);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  globals.display.width_px  = display_horizontal_px;
  globals.display.height_px = display_vertical_px;
  disp_drv.hor_res          = display_horizontal_px;
  disp_drv.ver_res          = display_vertical_px;
  disp_drv.flush_cb         = disp_flush;
  disp_drv.draw_buf         = &draw_buf_dsc_1;

  lv_disp_drv_register(&disp_drv);
}

//! Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
void disp_enable_update(void) { disp_flush_enabled = true; }

//! Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
void disp_disable_update(void) { disp_flush_enabled = false; }

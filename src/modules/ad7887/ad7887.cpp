#include "ad7887.h"

#include "ad7887_hw_config.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <pico/binary_info.h>

struct PwmSettings
{
  pwm_config cfg{};
  uint       slice{ 0 };
};

struct DmaSettings
{
  uint32_t           channel{ UINT32_MAX };
  dma_channel_config config{};
};

struct DmaPeriphery
{
  DmaSettings pwm_wrap;
  DmaSettings tx;
  DmaSettings rx;
  DmaSettings rx_ready;
};

struct DmaData
{
  ad7887::TransmissionData16b data_out{};
  Ad7887Sample               *data_in{ nullptr };
};

static PwmSettings  pwm_periphery{};
static DmaPeriphery dma_periphery{};
static DmaData      dma_data{};

static void gpio_init()
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("SPI Power Meter (ADC AD7887)"))
  bi_decl_if_func_used(bi_4pins_with_names(
    AD7887_GPIO_CS, "chip select, low active",
    AD7887_GPIO_SCLK, "SCLK",
    AD7887_GPIO_DIN, "probe input",
    AD7887_GPIO_DOUT, "probe output"))
    // clang-format on

    constexpr uint8_t out_gpios[]{ AD7887_GPIO_CS, AD7887_GPIO_SCLK, AD7887_GPIO_DIN };

  for (auto gpio : out_gpios)
  {
    gpio_init(gpio);
    gpio_set_pulls(gpio, false, false);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_SLOW);
    gpio_put(gpio, true);
  }
  // gpio_put(AD7887_GPIO_CS, true);
  gpio_put(AD7887_GPIO_CS, false);
  gpio_put(AD7887_GPIO_DIN, false);

  constexpr uint8_t in_gpios[]{ AD7887_GPIO_DOUT };

  for (auto gpio : in_gpios)
  {
    gpio_init(gpio);
    gpio_set_pulls(gpio, true, false);   // pull-up/down only slightly improves noise situation on ADC input signal
  }
}

static void pwm_init(PwmSettings &settings)
{
  constexpr uint8_t gpio{ 14 };

  settings.cfg = pwm_get_default_config();
  pwm_config_set_clkdiv_mode(&settings.cfg, PWM_DIV_FREE_RUNNING);
  pwm_config_set_clkdiv_int_frac(&settings.cfg, 3, 0);   // 150MHz / 3 = 50MHz
  pwm_config_set_wrap(&settings.cfg, 50000);             // 50MHz / 50000 = 1kH

  settings.slice = pwm_gpio_to_slice_num(gpio);
  pwm_init(settings.slice, &settings.cfg, false);

  pwm_set_chan_level(settings.slice, pwm_gpio_to_channel(gpio), 50000 / 2);
  gpio_set_function(gpio, GPIO_FUNC_PWM);
}

static void pwm_start(PwmSettings &settings) { pwm_set_enabled(settings.slice, true); }

static void spi_init()
{
#define AD7887_DISPLAY_SPI_INIT 1
#if AD7887_DISPLAY_SPI_INIT == 1
  const uint32_t spi_baud{ spi_init(AD7887_SPI_PORT, 500 * 1000) };
#else
  const uint32_t spi_baud{ spi_init(AD7887_SPI_PORT, 2 * 1000 * 1000) };
#endif
  spi_set_format(AD7887_SPI_PORT, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
  printf("ad7887 spi_baud=%" PRIu32 "\n", spi_baud);

  // gpio_set_function(AD7887_GPIO_CS, GPIO_FUNC_SPI);
  gpio_set_function(AD7887_GPIO_SCLK, GPIO_FUNC_SPI);
  gpio_set_function(AD7887_GPIO_DOUT, GPIO_FUNC_SPI);
  gpio_set_function(AD7887_GPIO_DIN, GPIO_FUNC_SPI);
}

static void on_trx_dma_finished_cb()
{
  if (dma_channel_get_irq1_status(dma_periphery.rx.channel))
  {
    dma_channel_acknowledge_irq1(dma_periphery.rx.channel);
    dma_data.data_in->is_data_ready = true;
    return;
  }
  else printf("W: unhandled irq\n");
}

static void rx_ready_dma_init(DmaSettings &settings, Ad7887Sample &data_out)
{
  settings.config = dma_channel_get_default_config(settings.channel);

  channel_config_set_transfer_data_size(&settings.config, DMA_SIZE_8);
  channel_config_set_read_increment(&settings.config, false);
  channel_config_set_write_increment(&settings.config, false);
  channel_config_set_chain_to(&settings.config, dma_periphery.pwm_wrap.channel);
  channel_config_set_high_priority(&settings.config, true);

  static const uint32_t is_data_ready{ true };
  dma_channel_configure(
    settings.channel,          // channel
    &settings.config,          // config
    &data_out.is_data_ready,   // write address
    &is_data_ready,            // read address
    1,                         // transaction count
    false);                    // trigger
}

static void pwm_dma_init(DmaSettings &settings, PwmSettings &pwm_settings)
{
  settings.config = dma_channel_get_default_config(settings.channel);

  channel_config_set_transfer_data_size(&settings.config, DMA_SIZE_32);
  channel_config_set_read_increment(&settings.config, false);
  channel_config_set_write_increment(&settings.config, false);
  channel_config_set_dreq(&settings.config, pwm_get_dreq(pwm_settings.slice));
  channel_config_set_high_priority(&settings.config, true);

  static const uint32_t trx_dma_trigger_mask{ 1u << dma_periphery.tx.channel | 1u << dma_periphery.rx.channel };
  dma_channel_configure(
    settings.channel,                 // channel
    &settings.config,                 // config
    &dma_hw->multi_channel_trigger,   // write address
    &trx_dma_trigger_mask,            // read address
    1,                                // transaction count
    false);                           // trigger
}

static void tx_dma_init(DmaSettings &settings, uint16_t &data_out)
{
  settings.config = dma_channel_get_default_config(settings.channel);

  channel_config_set_transfer_data_size(&settings.config, DMA_SIZE_16);
  channel_config_set_read_increment(&settings.config, false);
  channel_config_set_write_increment(&settings.config, false);
  channel_config_set_dreq(&settings.config, spi_get_dreq(AD7887_SPI_PORT, true));
  channel_config_set_chain_to(&settings.config, dma_periphery.rx_ready.channel);
  channel_config_set_high_priority(&settings.config, true);

  dma_channel_configure(
    settings.channel,                   // channel
    &settings.config,                   // config
    &spi_get_hw(AD7887_SPI_PORT)->dr,   // write address
    &data_out,                          // read address
    1,                                  // transaction count
    false);                             // trigger
}

static void rx_dma_init(DmaSettings &settings, uint16_t &data_in)
{
  settings.config = dma_channel_get_default_config(settings.channel);

  channel_config_set_transfer_data_size(&settings.config, DMA_SIZE_16);
  channel_config_set_read_increment(&settings.config, false);
  channel_config_set_write_increment(&settings.config, false);
  channel_config_set_dreq(&settings.config, spi_get_dreq(AD7887_SPI_PORT, false));
  channel_config_set_high_priority(&settings.config, true);

  dma_channel_configure(
    settings.channel,                   // channel
    &settings.config,                   // config
    &data_in,                           // write address
    &spi_get_hw(AD7887_SPI_PORT)->dr,   // read address
    1,                                  // transaction count
    false);                             // trigger

  dma_channel_set_irq1_enabled(settings.channel, true);
}

void ad7887_init(Ad7887Sample &data)
{
  // Setup:
  //  ┌──────────────────────────────────────────┐
  //  │          ADC-config → ────┐              │↑
  //  │↓                   ┌─ → TX-DMA → ─── → TX-DMA-ready
  //  │                    │     ↓│              │↓
  //  └── PWM-wrap-DMA → ──┤     SPI             │
  //           │           │     ↓│              └─ → sample_ready
  //           │↑          └─ → RX-DMA → ────────── → sample
  //      PWM-wrap DREQ

  dma_data.data_in = &data;

  gpio_init();
  spi_init();
  pwm_init(pwm_periphery);

  dma_periphery.tx.channel       = dma_claim_unused_channel(true);
  dma_periphery.rx.channel       = dma_claim_unused_channel(true);
  dma_periphery.rx_ready.channel = dma_claim_unused_channel(true);
  dma_periphery.pwm_wrap.channel = dma_claim_unused_channel(true);

  tx_dma_init(dma_periphery.tx, dma_data.data_out.asUint16);
  rx_dma_init(dma_periphery.rx, dma_data.data_in->data.asUint16);
  rx_ready_dma_init(dma_periphery.rx_ready, *dma_data.data_in);
  pwm_dma_init(dma_periphery.pwm_wrap, pwm_periphery);

  irq_set_exclusive_handler(DMA_IRQ_1, on_trx_dma_finished_cb);
  irq_set_enabled(DMA_IRQ_1, true);
}

void ad7887_start()
{
  // gpio_put(AD7887_GPIO_CS, false);
  pwm_start(pwm_periphery);
  dma_start_channel_mask(1u << dma_periphery.pwm_wrap.channel);
}

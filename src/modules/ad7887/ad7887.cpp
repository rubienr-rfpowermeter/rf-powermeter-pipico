#include "ad7887.h"

#include "ad7887_hw_config.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/binary_info.h>

struct DmaSettings
{
  uint32_t           channel{ UINT32_MAX };
  dma_channel_config config{};
};

struct DmaPeriphery
{
  DmaSettings tx;
  DmaSettings rx;

  ad7887::TransmissionData16b data_out{};
  Ad7887Sample               *data_in{ nullptr };
};

static DmaPeriphery dma_periphery{};

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

    constexpr uint8_t out_gpios[] = { AD7887_GPIO_CS, AD7887_GPIO_SCLK, AD7887_GPIO_DIN };

  for (auto gpio : out_gpios)
  {
    gpio_init(gpio);
    gpio_set_pulls(gpio, false, false);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_SLOW);
    gpio_put(gpio, true);
  }
  gpio_put(AD7887_GPIO_CS, true);
  gpio_put(AD7887_GPIO_DIN, false);

  constexpr uint8_t in_gpios[] = { AD7887_GPIO_DOUT };

  for (auto gpio : in_gpios)
  {
    gpio_init(gpio);
    gpio_set_pulls(gpio, false, true);
  }
}

static void __unused spi_init()
{
#define AD7887_DISPLAY_SPI_INIT 1
#if AD7887_DISPLAY_SPI_INIT == 1
  const uint32_t spi_baud = { spi_init(AD7887_SPI_PORT, 500 * 1000) };
#else
  const uint32_t spi_baud = { spi_init(AD7887_SPI_PORT, 2 * 1000 * 1000) };
#endif
  spi_set_format(AD7887_SPI_PORT, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
  printf("ad7887 spi_baud=%" PRIu32 "\n", spi_baud);

  gpio_set_function(AD7887_GPIO_CS, GPIO_FUNC_SPI);
  gpio_set_function(AD7887_GPIO_SCLK, GPIO_FUNC_SPI);
  gpio_set_function(AD7887_GPIO_DOUT, GPIO_FUNC_SPI);
  gpio_set_function(AD7887_GPIO_DIN, GPIO_FUNC_SPI);
}

__unused static void on_trx_dma_finished_cb()
{

  if (dma_channel_get_irq1_status(dma_periphery.tx.channel))
  {
    dma_channel_acknowledge_irq1(dma_periphery.tx.channel);
    gpio_put(AD7887_GPIO_CS, true);
  }

  if (dma_channel_get_irq1_status(dma_periphery.rx.channel))
  {
    dma_channel_acknowledge_irq1(dma_periphery.rx.channel);
    dma_periphery.data_in->is_data_ready = true;
  }
}

static void on_tx_dma_flush_cb()
{
  dma_periphery.data_in->is_data_ready = false;
  gpio_put(AD7887_GPIO_CS, false);
  dma_start_channel_mask(1u << dma_periphery.tx.channel | 1u << dma_periphery.rx.channel);
}

static void __unused tx_dma_init(DmaSettings &settings, uint16_t &data_out)
{
  settings.channel = dma_claim_unused_channel(true);

  settings.config = dma_channel_get_default_config(settings.channel);
  channel_config_set_transfer_data_size(&settings.config, DMA_SIZE_16);
  channel_config_set_read_increment(&settings.config, false);
  channel_config_set_write_increment(&settings.config, false);
  channel_config_set_dreq(&settings.config, spi_get_dreq(AD7887_SPI_PORT, true));

  dma_channel_configure(
    settings.channel,                   // channel
    &settings.config,                   // config
    &spi_get_hw(AD7887_SPI_PORT)->dr,   // write address
    &data_out,                          // read address
    1,                                  // transaction count
    false);                             // trigger

  dma_channel_set_irq1_enabled(settings.channel, true);
}

static void __unused rx_dma_init(DmaSettings &settings, uint16_t &data_in)
{
  settings.channel = dma_claim_unused_channel(true);
  settings.config  = dma_channel_get_default_config(settings.channel);

  channel_config_set_transfer_data_size(&settings.config, DMA_SIZE_16);
  channel_config_set_read_increment(&settings.config, false);
  channel_config_set_write_increment(&settings.config, false);
  channel_config_set_dreq(&settings.config, spi_get_dreq(AD7887_SPI_PORT, false));

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
  dma_periphery.data_in = &data;

  gpio_init();
  spi_init();

  tx_dma_init(dma_periphery.tx, dma_periphery.data_out.asUint16);
  rx_dma_init(dma_periphery.rx, dma_periphery.data_in->data.asUint16);
  irq_set_exclusive_handler(DMA_IRQ_1, on_trx_dma_finished_cb);
  irq_set_enabled(DMA_IRQ_1, true);
}

void ad7887_update() { on_tx_dma_flush_cb(); }

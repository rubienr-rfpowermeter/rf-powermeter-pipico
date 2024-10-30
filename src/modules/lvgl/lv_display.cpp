#include "lv_display.h"

#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
#include <cstdlib>
#include <hardware/gpio.h>
#include <lvgl.h>

struct LvglData
{
  uint8_t      *buffer_0;
  uint8_t      *buffer_1;
  lv_display_t *display;

  DmaPeriphery *dma_periphery;

  LvglData() : buffer_0(nullptr), buffer_1(nullptr), display(nullptr), dma_periphery(nullptr) { }
};

static LvglData lvgl_data;

static void on_dma_finished_handler()
{
  if (dma_channel_get_irq0_status(lvgl_data.dma_periphery->tx_dma_channel))
  {
    dma_channel_acknowledge_irq0(lvgl_data.dma_periphery->tx_dma_channel);
    gpio_put(DISPLAY_GPIO_CS, true);
    lv_display_flush_ready(lvgl_data.display);
  }
}

#include <cinttypes>
#include <cstdio>

static void display_flush_cb(lv_display_t __unused *display, const lv_area_t *area, uint8_t *px_map)
{
  display_set_window(area->x1, area->y1, area->x2 + 1, area->y2 + 1);

  const int32_t width  = { area->x2 - area->x1 + 1 };
  const int32_t height = { area->y2 - area->y1 + 1 };

  const uint16_t *in  = { (uint16_t *)(px_map) };
  uint16_t       *out = { (uint16_t *)lvgl_data.buffer_1 };

  for (int32_t x_px_map = area->x1; x_px_map <= area->x2; x_px_map++)
  {
    const int32_t x_buffer = { x_px_map - area->x1 };
    for (int32_t y_px_map = area->y1; y_px_map <= area->y2; y_px_map++)
    {
      const int32_t y_buffer           = { y_px_map - area->y1 };
      out[x_px_map + y_px_map * width] = in[x_buffer + y_buffer * width];
    }
  }

  gpio_put(DISPLAY_GPIO_DC, true);
  gpio_put(DISPLAY_GPIO_CS, false);

  dma_channel_hw_addr(lvgl_data.dma_periphery->tx_dma_channel)->read_addr = (uintptr_t)lvgl_data.buffer_1;
  dma_channel_hw_addr(lvgl_data.dma_periphery->tx_dma_channel)->al1_transfer_count_trig =
    width * height * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565);
}

static void lvgl_deinit()
{
  lv_deinit();
  free(lvgl_data.buffer_0);
  lvgl_data.buffer_0 = nullptr;
  free(lvgl_data.buffer_1);
  lvgl_data.buffer_1 = nullptr;
}

void lvgl_init()
{
  lv_init();

  constexpr size_t buffer_size_bytes = { DISPLAY_NUM_PIXELS * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565) };
  if (nullptr == lvgl_data.buffer_0) lvgl_data.buffer_0 = (uint8_t *)malloc(buffer_size_bytes);
  if (nullptr == lvgl_data.buffer_1) lvgl_data.buffer_1 = (uint8_t *)malloc(buffer_size_bytes);

  lvgl_data.display = lv_display_create(DISPLAY_HORIZONTAL_PX, DISPLAY_VERTICAL_PX);
  lv_display_set_buffers(lvgl_data.display, lvgl_data.buffer_0, nullptr, buffer_size_bytes, LV_DISPLAY_RENDER_MODE_FULL);
  lv_display_set_flush_cb(lvgl_data.display, display_flush_cb);

  irq_set_exclusive_handler(DMA_IRQ_0, on_dma_finished_handler);
  irq_set_enabled(DMA_IRQ_0, true);
}

void lv_display_init()
{
  lvgl_data.dma_periphery = &display_get_dma_periphery();
  lvgl_init();
}

void lv_display_deinit() { lvgl_deinit(); }

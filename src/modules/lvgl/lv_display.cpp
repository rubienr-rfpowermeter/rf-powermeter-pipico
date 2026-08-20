#include "lv_display.h"

#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
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

// About 1/10 of the display lets LVGL render one chunk while DMA transfers the
// other without reserving two complete frame buffers.
static constexpr size_t DRAW_BUFFER_LINE_COUNT { DISPLAY_HEIGHT_PX >= 10 ? DISPLAY_HEIGHT_PX / 10 : 1 };
static constexpr size_t DRAW_BUFFER_SIZE_BYTES { DISPLAY_WIDTH_PX * DRAW_BUFFER_LINE_COUNT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565) };

static void on_dma_finished_handler()
{
  if (dma_channel_get_irq0_status(lvgl_data.dma_periphery->tx_dma_channel))
  {
    dma_channel_acknowledge_irq0(lvgl_data.dma_periphery->tx_dma_channel);
    gpio_put(DISPLAY_GPIO_CS, true);
    lv_display_flush_ready(lvgl_data.display);
  }
}

static void display_flush_cb(lv_display_t __unused *display, const lv_area_t *area, uint8_t *px_map)
{
  display_set_window(area->x1, area->y1, area->x2 + 1, area->y2 + 1);

  const int32_t width { area->x2 - area->x1 + 1 };
  const int32_t height { area->y2 - area->y1 + 1 };

  gpio_put(DISPLAY_GPIO_DC, true);
  gpio_put(DISPLAY_GPIO_CS, false);

  dma_channel_hw_addr(lvgl_data.dma_periphery->tx_dma_channel)->read_addr               = (uintptr_t)px_map;
  dma_channel_hw_addr(lvgl_data.dma_periphery->tx_dma_channel)->al1_transfer_count_trig = width * height * sizeof(uint16_t);
}

static void lvgl_deinit()
{
  lv_deinit();
  lv_free(lvgl_data.buffer_0);
  lvgl_data.buffer_0 = nullptr;
  lv_free(lvgl_data.buffer_1);
  lvgl_data.buffer_1 = nullptr;
}

void lvgl_init()
{
  lv_init();

  if (nullptr == lvgl_data.buffer_0) lvgl_data.buffer_0 = (uint8_t *)lv_malloc(DRAW_BUFFER_SIZE_BYTES);
  if (nullptr == lvgl_data.buffer_1) lvgl_data.buffer_1 = (uint8_t *)lv_malloc(DRAW_BUFFER_SIZE_BYTES);

  LV_ASSERT_NULL(lvgl_data.buffer_0);
  LV_ASSERT_NULL(lvgl_data.buffer_1);

  lvgl_data.display = lv_display_create(DISPLAY_WIDTH_PX, DISPLAY_HEIGHT_PX);
  lv_display_set_color_format(lvgl_data.display, LV_COLOR_FORMAT_RGB565_SWAPPED);
  lv_display_set_buffers(lvgl_data.display, lvgl_data.buffer_0, lvgl_data.buffer_1, DRAW_BUFFER_SIZE_BYTES, LV_DISPLAY_RENDER_MODE_PARTIAL);
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

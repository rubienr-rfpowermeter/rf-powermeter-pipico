#include "lv_display.h"

#include "hardware/gpio.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
#include <cstdlib>

typedef struct LvglData
{
  lv_disp_draw_buf_t disp_buffer;
  lv_color_t *buffer_0;
  lv_color_t *buffer_1;
  lv_disp_drv_t disp_diver;

  DmaPeriphery *dma_periphery;

  LvglData() : buffer_0(nullptr), buffer_1(nullptr), disp_diver(0), dma_periphery(nullptr) { }
} LvglData;

static LvglData lvgl_data;


static void on_dma_finished_handler()
{
  if(dma_channel_get_irq0_status(lvgl_data.dma_periphery->tx_dma_channel))
  {
    dma_channel_acknowledge_irq0(lvgl_data.dma_periphery->tx_dma_channel);
    gpio_put(DISPLAY_GPIO_CS, true);
    lv_disp_flush_ready(&lvgl_data.disp_diver);
  }
}

static void dispay_flush_cb(__unused lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *color_p)
{
  display_set_window(area->x1, area->y1, area->x2 + 1, area->y2 + 1);

  gpio_put(DISPLAY_GPIO_DC, true);
  gpio_put(DISPLAY_GPIO_CS, false);

  // set read address, no trigger
  dma_channel_hw_addr(lvgl_data.dma_periphery->tx_dma_channel)->read_addr = (uintptr_t)color_p;
  // set write count and trigger
  const uint32_t tx_bytes_count = { ((area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1)) * 2u };
  dma_channel_hw_addr(lvgl_data.dma_periphery->tx_dma_channel)->al1_transfer_count_trig = tx_bytes_count;
}

static void lvgl_deinit()
{
  free(lvgl_data.buffer_0);
  lvgl_data.buffer_0 = nullptr;
  free(lvgl_data.buffer_1);
  lvgl_data.buffer_1 = nullptr;
}

void lvgl_init(DmaPeriphery &dma_periphery)
{
  lv_init();

  if(nullptr == lvgl_data.buffer_0) lvgl_data.buffer_0 = (lv_color_t *)malloc(DISPLAY_NUM_PIXELS / 2 * sizeof(lv_color_t));
  if(nullptr == lvgl_data.buffer_1) lvgl_data.buffer_1 = (lv_color_t *)malloc(DISPLAY_NUM_PIXELS / 2 * sizeof(lv_color_t));

  lv_disp_draw_buf_init(&lvgl_data.disp_buffer, lvgl_data.buffer_0, lvgl_data.buffer_1, DISPLAY_NUM_PIXELS / 2);
  lv_disp_drv_init(&lvgl_data.disp_diver);

  lvgl_data.disp_diver.hor_res  = DISPLAY_WIDTH_PX;
  lvgl_data.disp_diver.ver_res  = DISPLAY_HEIGHT_PX;
  lvgl_data.disp_diver.flush_cb = dispay_flush_cb;
  lvgl_data.disp_diver.draw_buf = &lvgl_data.disp_buffer;
  lv_disp_drv_register(&lvgl_data.disp_diver);

  irq_set_exclusive_handler(DMA_IRQ_0, on_dma_finished_handler);
  irq_set_enabled(DMA_IRQ_0, true);
}

void lv_display_init()
{
  lvgl_data.dma_periphery = &display_get_dma_periphery();
  lvgl_init(*lvgl_data.dma_periphery);
}

void lv_display_deinit() { lvgl_deinit(); }

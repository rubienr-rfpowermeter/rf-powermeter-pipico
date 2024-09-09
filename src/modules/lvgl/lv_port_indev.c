#include "lv_port_indev.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lvgl.h"
#include "lib/types/cast.h"
#include "modules/globals/globals.h"

#include <pico/binary_info/code.h>
#include <stdio.h>

constexpr static bool gt911_invert_x                  = { false };
constexpr static bool gt911_invert_y                  = { false };
constexpr static bool gt911_swap_xy                   = { true };
constexpr static bool gt911_enable_sw_noise_reduction = { true };


static constexpr uint16_t gt911_resolution_x            = { 480 };
static constexpr uint16_t gt911_resolution_y            = { 320 };
static constexpr bool gt911_use_alternative_i2c_address = { false };
static constexpr uint16_t gt911_max_touch_supported     = { 5 };

static constexpr uint8_t gt911_gpio_sda = { 8 };
static constexpr uint8_t gt911_gpio_scl = { 9 };
static constexpr uint8_t gt911_gpio_rst = { 10 };
static constexpr uint8_t gt911_gpio_int = { 11 };

lv_indev_t *indev_touchpad = { nullptr };
static constexpr uint8_t gt911_i2c_dev_addr = { gt911_use_alternative_i2c_address ? GT911_I2C_SLAVE_ADDR_ALTERNATIVE : GT911_I2C_SLAVE_ADDR_DEFAULT };

static void compute_config_checksum(const uint8_t *gt911_config, uint8_t len, uint8_t *crc_dest)
{
  *crc_dest = 0;
  for(uint8_t idx = 0; idx < len; idx++)
    *crc_dest = *crc_dest + gt911_config[idx];
  *crc_dest = (~(*crc_dest)) + 1;
}

static int gt911_i2c_read(uint8_t slave_addr, uint16_t register_address, uint8_t *data_buf, uint8_t len)
{
  uint8_t reg_addr[] = { (register_address & 0xFF00) >> 8, register_address & 0x00FF };
  if(i2c_write_blocking(i2c0, slave_addr, reg_addr, 2, true)) return i2c_read_blocking(i2c0, slave_addr, data_buf, len, false);
  else return PICO_ERROR_GENERIC;
}

static int gt911_i2c_write(uint16_t slave_addr, uint16_t register_address, const uint8_t *data_buf, uint8_t len)
{
  // assume the largest buffer to write is the whole configuration of 186 byte

  uint8_t buffer[186 + 2] = { [0] = (register_address & 0xFF00) >> 8, [1] = register_address & 0x00FF };
  if(len > sizeof(buffer) - 2) return PICO_ERROR_GENERIC;
  memcpy(&buffer[2], data_buf, len);

  return i2c_write_blocking(i2c0, slave_addr, buffer, len + 2, true);
}

//! Will be called by the library to read the touchpad
static void gt911_read_cb(lv_indev_drv_t __unused *indev_drv, lv_indev_data_t *data)
{
  static int16_t last_x = { 0 }; // 12bit pixel value
  static int16_t last_y = { 0 }; // 12bit pixel value

  Gt911_StatusRegister status_reg;
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_STATUS_REG, &status_reg.data, 1);

  // clear status register
  constexpr uint8_t reset = { 0x00 };
  gt911_i2c_write(gt911_i2c_dev_addr, GT911_STATUS_REG, &reset, 1);

  data->btn_id           = LV_INDEV_TYPE_POINTER;
  data->continue_reading = false;
  data->point.x          = last_x;
  data->point.y          = last_y;

  if(!status_reg.is_buffer_ok) return;

  data->state = (status_reg.touch_count < 1) ? LV_INDEV_STATE_RELEASED : LV_INDEV_STATE_PRESSED;

  IntCast *x_in = { (IntCast *)&last_x };
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_PT1_X_COORD_L, &(x_in->as_8bit.b0.uint8), 1);
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_PT1_X_COORD_H, &(x_in->as_8bit.b1.uint8), 1);

  IntCast *y_in = { (IntCast *)&last_y };
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_PT1_Y_COORD_L, &(y_in->as_8bit.b0.uint8), 1);
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_PT1_Y_COORD_H, &(y_in->as_8bit.b1.uint8), 1);

  data->point.x = last_x;
  data->point.y = last_y;

  printf(
    "xxx x=%03hd y=%03hd | rdy=%hhu lar=%hhu prx=%hhu kpr=%hhu pts=%hhu\n", data->point.x, data->point.y, status_reg.is_buffer_ok,
    status_reg.is_large_area_detected, status_reg.is_proximity, status_reg.have_key, status_reg.touch_count);
  data->state = LV_INDEV_STATE_PRESSED;
}

static void gt911_send_config(void)
{
  // clang-format off
  uint8_t default_gt911_config[GT911_CFG_END_ADDRESS - GT911_CFG_BASE_ADDRESS + 1] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x05, // 0x8047 - Config_Version, X_Output_Max_L, X_Output_Max_H, Y_Output_Max_L, Y_Output_Max_H, Touch_Number
    0x0C, 0x20, 0x01, 0x08, 0x28, 0x05, // 0x804D - Module_Switch1, Module_Switch2, Shake_Count, Filter, Large_Touch, Noise_Reduction
    0x50, 0x3C, 0x0F, 0x05, 0x00, 0x00, // 0x8053 - Screen_Touch_Level, Screen_Leave_Level, Low_Power_Control, Refresh_Rate, X_Threshold, Y_Threshold
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8059 - X_Speed_Limit, Y_Speed_Limit, Space, Space, Mini_Filter, Stretch_R0
    0x00, 0x00, 0x00, 0x89, 0x2A, 0x0B, // 0x805F - Stretch_R1, Stretch_R2, Stretch_RM, Drv_GroupA_Num, Drv_GroupB_Num, Sensor_Num
    0x2D, 0x2B, 0x0F, 0x0A, 0x00, 0x00, // 0x8065 - FreqA_Factor, FreqB_Factor, Panel_BitFreqL, Panel_BitFreqH, Panel_Sensor_TimeL (res.), Panel_Sensor_TimeH (res.)
    0x01, 0xA9, 0x03, 0x2D, 0x00, 0x01, // 0x806B - Panel_Tx_Gain, Panel_Rx_Gain, Panel_Dump_Shift, Drv_Frame_Control, Charging_Level_Up, Module_Switch3
    0x00, 0x00, 0x00, 0x03, 0x00, 0x00, // 0x8071 - Gesture_Dis, Gesture_Long_Press_Time, XY_Slope_Adjust, Gesture_Control, Gesture_Switch1, Gesture_Switch2
    0x00, 0x00, 0x00, 0x21, 0x59, 0x94, // 0x8077 - Gesture_Refresh_Rate, Gesture_Touch_Level, New_Green_Wakeup_Level, Freq_Hopping_Start, Freq_Hopping_End, Noise_Detect_Times
    0xC5, 0x02, 0x07, 0x00, 0x00, 0x04, // 0x807D - Hopping_Flag, Hopping_Threshold, Noise_Threshold (res.), NoiseMon_Threshold (res.), NC, Hopping_Sensor_Group
    0x93, 0x24, 0x00, 0x7D, 0x2C, 0x00, // 0x8083 - Hoping_seq1_Normalize, Hopping_seq1_Factor, Main_Clock_Adjust, Hoping_seq2_Normalize, Hopping_seq2_Factor, NC
    0x6B, 0x36, 0x00, 0x5D, 0x42, 0x00, // 0x8089 - Hoping_seq3_Normalize, Hopping_seq3_Factor, NC, Hoping_seq4_Normalize, Hopping_seq4_Factor, NC
    0x53, 0x50, 0x00, 0x53, 0x00, 0x00, // 0x808F - Hopping_seq5_Factor, NC, Hopping_seq6_Normalize, Key1, Key2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8095 - Key3, Key4, Key_Area, Key_Touch_Level, Key_Leave_Level, Key_Sens,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x809B - Key_Sens, Key_Restrain, Key_Restrain_Time, Gesture_Large_Touch, NC, NC
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80A1 - Hotknot_Noise_Map, Link_threshold, Pxy_Threshold, GHot_Dmp_Shift, GHot_Rx_Gain, Freq_Gain0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80A7 - Freq_Cain1, Freq_Gain2, FreqGain3, NC, NC, NC
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80AD - NC, NC, NC, NC, NC, NC

    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // 0x80B3 - Combine_Dis, Split_Set, NC, NC, B7-C4 Sensor_CH0-Sensor_CH13
    0x04, 0x06, 0x08, 0x0A, 0x0c, 0x0E, // 0x80B9 - B7-C4 Sensor_CH0-Sensor_CH13
    0x10, 0x12, 0xFF, 0xFF, 0xFF, 0xFF, // 0x80BF - B7-C4 Sensor_CH0-Sensor_CH13

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80C5 - NC, NC, NC, NC, NC, NC
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80CB - NC, NC, NC, NC, NC, NC

    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // 0x80D1 - NC, NC, NC, NC, D5-EE Driver_CH0-Driver_CH25
    0x04, 0x06, 0x08, 0x0A, 0x0B, 0x0c, // 0x80D7 - D5-EE Driver_CH0-Driver_CH25
    0x24, 0x22, 0x21, 0x20, 0x1f, 0x1e, // 0x80DD - D5-EE Driver_CH0-Driver_CH25
    0x1d, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x80E3 - D5-EE Driver_CH0-Driver_CH25
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0x80E9 - D5-EE Driver_CH0-Driver_CH25

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80EF - NC, NC, NC, NC, NC, NC
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80F5 - NC, NC, NC, NC, NC, NC
    0x00, 0x00, 0x00, 0x00,             // 0x80FB - NC, NC, NC, NC, NC, NC
    0x00,                               // 0x80FF - config_crc
    0x00};                              // 0x8100 - config fresh
  // clang-format on

  default_gt911_config[GT911_CFG_OUT_MAX_X_L - GT911_CFG_BASE_ADDRESS] = gt911_resolution_x & 0x00FF;
  default_gt911_config[GT911_CFG_OUT_MAX_X_H - GT911_CFG_BASE_ADDRESS] = (gt911_resolution_x & 0xFF00) >> 8;

  default_gt911_config[GT911_CFG_OUT_MAX_Y_L - GT911_CFG_BASE_ADDRESS] = gt911_resolution_y & 0x00FF;
  default_gt911_config[GT911_CFG_OUT_MAX_Y_H - GT911_CFG_BASE_ADDRESS] = (gt911_resolution_y & 0xFF00) >> 8;

  default_gt911_config[GT911_CFG_TOUCH_NUMBER - GT911_CFG_BASE_ADDRESS] = gt911_max_touch_supported;

  if(gt911_invert_x)
    default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] |= GT911_CFG_MODULE_SWITCH1_INVERT_X_FLAG;
  else default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] &= ~GT911_CFG_MODULE_SWITCH1_INVERT_X_FLAG;

  if(gt911_invert_y)
    default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] |= GT911_CFG_MODULE_SWITCH1_INVERT_Y_FLAG;
  else default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] &= ~GT911_CFG_MODULE_SWITCH1_INVERT_Y_FLAG;

  if(gt911_swap_xy)
    default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] |= GT911_CFG_MODULE_SWITCH1_SWAP_XY_FLAG;
  else default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] &= ~GT911_CFG_MODULE_SWITCH1_SWAP_XY_FLAG;

  if(gt911_enable_sw_noise_reduction)
    default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] |= GT911_CFG_MODULE_SWITCH1_SOFTWARE_NOISE_REDUCTION_FLAG;
  else
    default_gt911_config[GT911_CFG_MODULE_SWITCH1 - GT911_CFG_BASE_ADDRESS] &= ~GT911_CFG_MODULE_SWITCH1_SOFTWARE_NOISE_REDUCTION_FLAG;

  // take over device's current config_version
  constexpr uint8_t device_version_idx = { GT911_CFG_CFG_VERSION - GT911_CFG_BASE_ADDRESS };
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_CFG_CFG_VERSION, &default_gt911_config[device_version_idx], 1);

  // compute CRC of potential new configuration
  constexpr uint8_t checksum_idx = { GT911_CFG_CHECKSUM - GT911_CFG_BASE_ADDRESS };
  compute_config_checksum(default_gt911_config, checksum_idx, &default_gt911_config[checksum_idx]);
  const uint8_t config_crc = { default_gt911_config[checksum_idx] };

  // device config CRC
  uint8_t device_config_crc;
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_CFG_CHECKSUM, &device_config_crc, 1);

  if(config_crc == device_config_crc)
  {
    printf("lv_port_indev: device config is already up-to-date (crc match: 0x%02hhx )\n", config_crc);
    return;
  }

  uint8_t in_config[GT911_CFG_END_ADDRESS - GT911_CFG_BASE_ADDRESS + 1] = { 0 };
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_CFG_BASE_ADDRESS, in_config, sizeof(in_config));
  printf("lv_port_indev: config before update:\n");

  // demonstrate config
  for(size_t idx_in = 1; idx_in <= sizeof(default_gt911_config); idx_in++)
  {
    printf("%02hhX ", in_config[idx_in - 1]);
    if(0 == (idx_in % 6))
      printf(": 0x%04hX - 0x%04hX\n", GT911_CFG_BASE_ADDRESS + idx_in - 6, GT911_CFG_BASE_ADDRESS + idx_in - 1);
  }

  // persistently write new config to device
  printf("lv_port_indev: outdated device config detected (new config vs dev crc: 0x%02hhx vs 0x%02hhx) , updated started ...\n", config_crc, device_config_crc);
  gt911_i2c_write(gt911_i2c_dev_addr, GT911_CFG_BASE_ADDRESS, default_gt911_config, sizeof(default_gt911_config));

  printf("lv_port_indev: config sent, activating config ...\n");
  uint8_t fresh = { 1 };
  gt911_i2c_write(gt911_i2c_dev_addr, GT911_CFG_FRESH, &fresh, sizeof(fresh));

  // check transaction: re-read whole config from device
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_CFG_BASE_ADDRESS, in_config, sizeof(in_config));

  const uint8_t reported_crc_after_update = in_config[checksum_idx];
  compute_config_checksum(in_config, checksum_idx, &in_config[checksum_idx]);
  const uint8_t computed_crc_after_update = in_config[checksum_idx];

  if(reported_crc_after_update != computed_crc_after_update)
  {
    printf(
      "lv_port_indev: failed: fetched inconsistent config from device "
      "(reported vx computed device crc: 0x%02hhx 0x%02hhx)\n",
      reported_crc_after_update, computed_crc_after_update);
    return;
  }

  if(config_crc != reported_crc_after_update)
  {
    printf(
      "lv_port_indev: failed: fetched inconsistent config from device "
      "(reported vx computed crc: 0x%02hhx 0x%02hhx)\n",
      reported_crc_after_update, computed_crc_after_update);
    return;
  }

  printf("lv_prot_indev: config update finished (crc 0x%02hhx)\n", reported_crc_after_update);

  // demonstrate both configs
  int idx_in = 1;
  printf("lv_prot_indev:\ncfg sent to dev   | cfg stored on dev\n");
  for(size_t idx_out = 1; idx_out <= sizeof(default_gt911_config); idx_out++)
  {
    printf("%02hhX ", default_gt911_config[idx_out - 1]);
    if(0 == (idx_out % 6))
    {
      printf("| ");
      do
        printf("%02hhX ", in_config[idx_in - 1]);
      while(0 != (idx_in++ % 6));
      printf(": 0x%04hx - 0x%04hx\n", GT911_CFG_BASE_ADDRESS + idx_out - 6, GT911_CFG_BASE_ADDRESS + idx_out - 1);
    }
  }

  uint8_t reset = 0;
  gt911_i2c_write(gt911_i2c_dev_addr, GT911_COMMAND_REGISTER, &reset, 1);
}

static void gt911_restart_device(bool set_default_address)
{
  // prepare INT line: input, pull down
  gpio_pull_down(gt911_gpio_int);
  gpio_put(gt911_gpio_int, 0);
  gpio_set_dir(gt911_gpio_int, GPIO_OUT);

  // reset: enable
  gpio_put(gt911_gpio_rst, 0);
  busy_wait_ms(10); // min hold time: not documented

  if(set_default_address) gpio_put(gt911_gpio_int, 0); // INT low: address 0x5D
  else gpio_put(gt911_gpio_int, 1);                    // INT high: address 0x14
  busy_wait_ms(10);                                    // min hold time: 100us

  // reset: disable
  gpio_put(gt911_gpio_rst, 1);
  busy_wait_ms(20); // min hold time: 5ms

  // leave INT as floating input
  gpio_disable_pulls(gt911_gpio_int);
  gpio_set_dir(gt911_gpio_int, GPIO_IN);
}

//! Initialize your touchpad
static void gt911_init(void)
{
  // clang-format off
  bi_decl_if_func_used(bi_program_feature("Capacitive Touch (I2C0)"))
  bi_decl_if_func_used(bi_2pins_with_func(gt911_gpio_sda, gt911_gpio_scl, GPIO_FUNC_I2C))
  bi_decl_if_func_used(bi_2pins_with_names(gt911_gpio_sda, "Touch", gt911_gpio_scl, "Touch"))
  bi_decl_if_func_used(bi_2pins_with_names(gt911_gpio_rst, "RST, Touch", gt911_gpio_int, "INT, Touch"))
    // clang-format on

    constexpr uint target_baud_rate = { 400 * 1000 };
  const uint effective_baud_rate    = { i2c_init(i2c0, target_baud_rate) };
  printf("lv_port_indev: i2c0 effective_clock_hz=%u requested_clock_hz=%u\n", effective_baud_rate, target_baud_rate);

  gpio_set_function(gt911_gpio_sda, GPIO_FUNC_I2C);
  gpio_set_function(gt911_gpio_scl, GPIO_FUNC_I2C);
  gpio_set_function(gt911_gpio_int, GPIO_FUNC_SIO);
  gpio_set_function(gt911_gpio_rst, GPIO_FUNC_SIO);
  gpio_set_dir(gt911_gpio_rst, GPIO_OUT);
  gpio_set_dir(gt911_gpio_int, GPIO_IN);

  gpio_pull_up(gt911_gpio_sda);
  gpio_pull_up(gt911_gpio_scl);
  gpio_disable_pulls(gt911_gpio_rst);
  gpio_disable_pulls(gt911_gpio_int);

  gt911_restart_device(!gt911_use_alternative_i2c_address);

  uint8_t dont_care;
  if(gt911_i2c_read(gt911_i2c_dev_addr, GT911_PRODUCT_ID1, &dont_care, 1) == PICO_ERROR_GENERIC) return;

  gt911_send_config();

  gt911_i2c_read(gt911_i2c_dev_addr, (GT911_PRODUCT_ID1), (uint8_t *)globals.touch.product_id, GT911_PRODUCT_ID_LEN);
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_VENDOR_ID, &globals.touch.vendor_id, 1);
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_X_COORD_RES_L, (uint8_t *)&globals.touch.width_px, 2);
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_Y_COORD_RES_L, (uint8_t *)&globals.touch.height_px, 2);
  gt911_i2c_read(gt911_i2c_dev_addr, GT911_FIRMWARE_VER_L, (uint8_t *)&globals.touch.firmware_version, 2);
  globals.touch.i2c_address = gt911_i2c_dev_addr;
  globals.touch.baud_rate   = effective_baud_rate;

  printf("lv_port_indev: touch initialized\n");
}

void lv_port_indev_init(void)
{
  // device init
  gt911_init();

  // register device to lvgl
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type    = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = gt911_read_cb;
  indev_touchpad    = lv_indev_drv_register(&indev_drv);

  // enable mouse pointer
  lv_obj_t *mouse_cursor = lv_img_create(lv_scr_act());
  lv_img_set_src(mouse_cursor, LV_SYMBOL_DUMMY "x");
  lv_indev_set_cursor(indev_touchpad, mouse_cursor);
}

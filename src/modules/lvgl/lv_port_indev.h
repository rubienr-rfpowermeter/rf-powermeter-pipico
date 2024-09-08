#pragma once

#include "lvgl.h"

#define GT911_I2C_SLAVE_ADDR_DEFAULT 0x5D
#define GT911_I2C_SLAVE_ADDR_ALTERNATIVE 0x14

#define GT911_PRODUCT_ID_LEN 4

#define GT911_COMMAND_REGISTER 0x8040

//! configuration register map
#define GT911_CFG_BASE_ADDRESS 0x8047 // first configuration register
#define GT911_CFG_CFG_VERSION 0x8047
#define GT911_CFG_OUT_MAX_X_L 0x8048
#define GT911_CFG_OUT_MAX_X_H 0x8049
#define GT911_CFG_OUT_MAX_Y_L 0x804A
#define GT911_CFG_OUT_MAX_Y_H 0x804B
#define GT911_CFG_TOUCH_NUMBER 0x804C
#define GT911_CFG_MODULE_SWITCH1 0x804D
#define GT911_CFG_MODULE_SWITCH1_INVERT_Y_FLAG (1 << 7)
#define GT911_CFG_MODULE_SWITCH1_INVERT_X_FLAG (1 << 6)
#define GT911_CFG_MODULE_SWITCH1_SWAP_XY_FLAG (1 << 3)
#define GT911_CFG_MODULE_SWITCH1_SOFTWARE_NOISE_REDUCTION_FLAG (1 << 2)
#define GT911_CFG_CHECKSUM 0x80FF    // configuration verification of 0x8047 to 0x80fE
#define GT911_CFG_FRESH 0x8100       // config update flag written by host
#define GT911_CFG_END_ADDRESS 0x8100 // last configuration register

//! output register map of GT911
#define GT911_PRODUCT_ID1 0x8140
#define GT911_FIRMWARE_VER_L 0x8144
#define GT911_FIRMWARE_VER_H 0x8145
#define GT911_X_COORD_RES_L 0x8146
#define GT911_X_COORD_RES_H 0x8147
#define GT911_Y_COORD_RES_L 0x8148
#define GT911_Y_COORD_RES_H 0x8149
#define GT911_VENDOR_ID 0x814A
#define GT911_STATUS_REG 0x814E
#define GT911_PT1_X_COORD_L 0x8150
#define GT911_PT1_X_COORD_H 0x8151
#define GT911_PT1_Y_COORD_L 0x8152
#define GT911_PT1_Y_COORD_H 0x8153

typedef union __packed
{
  uint8_t data;
  struct __packed
  {
    uint8_t touch_count : 4;
    uint8_t have_key : 1;
    uint8_t is_proximity : 1;
    uint8_t is_large_area_detected : 1;
    uint8_t is_buffer_ok : 1;
  };
} Gt911_StatusRegister;

static_assert(sizeof(Gt911_StatusRegister) == 1);

#ifdef __cplusplus
extern "C"
{
#endif

void lv_port_indev_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

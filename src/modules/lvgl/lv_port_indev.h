#pragma once

#include "lvgl.h"

#define GT911_I2C_SLAVE_ADDR_DEFAULT 0x5D
#define GT911_I2C_SLAVE_ADDR_ALTERNATIVE 0x14

#define GT911_PRODUCT_ID_LEN 4

//! configuration register map
#define GT911_COMMAND_REGISTER 0x8040

#define GT911_CFG_BASE_ADDRESS 0x8047
#define GT911_CFG_CFG_VERSION 0x8047
#define GT911_CFG_OUT_MAX_X_L 0x8048
#define GT911_CFG_OUT_MAX_X_H 0x8049
#define GT911_CFG_OUT_MAX_Y_L 0x804A
#define GT911_CFG_OUT_MAX_Y_H 0x804B
#define GT911_CFG_TOUCH_NUMBER 0x804C
#define GT911_CFG_MODULE_SWITCH1 0x804D
#define GT911_CFG_MODULE_SWITCH1_INVERT_Y (1 << 7)
#define GT911_CFG_MODULE_SWITCH1_INVERT_X (1 << 6)
#define GT911_CFG_MODULE_SWITCH1_SWAP_XY (1 << 3)
#define GT911_CFG_MODULE_SWITCH1_SOFTWARE_NOISE_REDUCTION (1 << 2)
#define GT911_CFG_CHECKSUM 0x80FF // configuration verification of 0x8047 to 0x80fE
#define GT911_CFG_FRESH 0x8100    // config update flag written by host

//! output register map of GT911
#define GT911_PRODUCT_ID1 0x8140
#define GT911_PRODUCT_ID2 0x8141
#define GT911_PRODUCT_ID3 0x8142
#define GT911_PRODUCT_ID4 0x8143
#define GT911_FIRMWARE_VER_L 0x8144
#define GT911_FIRMWARE_VER_H 0x8145
#define GT911_X_COORD_RES_L 0x8146
#define GT911_X_COORD_RES_H 0x8147
#define GT911_Y_COORD_RES_L 0x8148
#define GT911_Y_COORD_RES_H 0x8149
#define GT911_VENDOR_ID 0x814A

#define GT911_STATUS_REG 0x814E
#define GT911_STATUS_REG_COORDINATE_READY (1 << 7)
#define GT911_STATUS_REG_LARGE_AREA_TP_DETECTED (1 << 6)
#define GT911_STATUS_REG_IS_PROXIMITY_VALID (1 << 5)
#define GT911_STATUS_REG_IS_KEY_PRESSED (1 << 4)
#define GT911_STATUS_REG_NUM_TOUCH_PT_MASK 0b01111

#define GT911_TRACK_ID1 0x814F
#define GT911_PT1_X_COORD_L 0x8150
#define GT911_PT1_X_COORD_H 0x8151
#define GT911_PT1_Y_COORD_L 0x8152
#define GT911_PT1_Y_COORD_H 0x8153
#define GT911_PT1_X_SIZE_L 0x8154
#define GT911_PT1_X_SIZE_H 0x8155

typedef struct
{
    bool inited;
    char product_id[GT911_PRODUCT_ID_LEN];
    uint16_t max_x_coord;
    uint16_t max_y_coord;
    uint8_t i2c_dev_addr;
} gt911_status_t;

#ifdef __cplusplus
extern "C"
{
#endif

void lv_port_indev_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

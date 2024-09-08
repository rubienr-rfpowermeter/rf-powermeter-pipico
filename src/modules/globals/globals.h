#pragma once

#include <stdint.h>

typedef struct TouchInfo
{
    uint32_t product_id;
    uint16_t width_px;
    uint16_t height_px;
    uint8_t i2c_address;
    uint32_t baud_rate;
    uint16_t firmware_version;
    uint8_t vendor_id;
} TouchInfo;

typedef struct DisplayInfo
{
    uint16_t width_px;
    uint16_t height_px;
    uint32_t baud_rate;
} DisplayInfo;

typedef struct
{
    struct TouchInfo touch;
    struct DisplayInfo display;
} Globals;

extern Globals globals;

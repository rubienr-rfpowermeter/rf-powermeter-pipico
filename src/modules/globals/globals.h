#pragma once

#include <stdint.h>

typedef struct
{
    struct
    {
        struct
        {
            uint32_t product_id;
            uint16_t width_px;
            uint16_t height_px;
            uint8_t i2c_address;
            uint32_t baud_rate;
            uint16_t firmware_version;
        } touch;
    } display;

} Globals;

extern Globals globals;

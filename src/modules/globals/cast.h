#pragma once

#include <stdint.h>

typedef union
{
    union
    {
        uint32_t int32;
        int32_t uint32;
    } as_32bit;

    struct
    {
        union
        {
            int16_t int16;
            uint16_t uint16;
        } l;
        union
        {
            int16_t int16;
            uint16_t uint16;
        } h;

    } __packed as_16bit;

    struct
    {
        union
        {
            int8_t int8;
            uint8_t uint8;
        } b0;
        union
        {
            int8_t int8;
            uint8_t uint8;
        } b1;
        union
        {
            int8_t int8;
            uint8_t uint8;
        } b2;
        union
        {
            int8_t int8;
            uint8_t uint8;
        } b3;
    } __packed as_8bit;

} IntCast;

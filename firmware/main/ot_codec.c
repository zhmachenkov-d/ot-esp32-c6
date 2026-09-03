#include "ot_codec.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

float ot_codec_f88_to_float(uint16_t raw)
{
    int16_t s = (int16_t)raw;
    return ((float)s) / 256.0f;
}

uint16_t ot_codec_float_to_f88(float celsius)
{
    float scaled = celsius * 256.0f;
    if (scaled > 32767.0f) {
        scaled = 32767.0f;
    } else if (scaled < -32768.0f) {
        scaled = -32768.0f;
    }
    return (uint16_t)(int16_t)lroundf(scaled);
}

uint16_t ot_codec_pack_hb_lb(uint8_t hb, uint8_t lb)
{
    return ((uint16_t)hb << 8) | (uint16_t)lb;
}

void ot_codec_unpack_hb_lb(uint16_t raw, uint8_t *hb, uint8_t *lb)
{
    if (hb) {
        *hb = (uint8_t)((raw >> 8) & 0xFF);
    }
    if (lb) {
        *lb = (uint8_t)(raw & 0xFF);
    }
}

bool ot_codec_flag8_get(uint8_t flags, unsigned bit)
{
    if (bit > 7) {
        return false;
    }
    return (flags & (1u << bit)) != 0;
}

uint8_t ot_codec_flag8_set(uint8_t flags, unsigned bit, bool value)
{
    if (bit > 7) {
        return flags;
    }
    if (value) {
        return (uint8_t)(flags | (1u << bit));
    }
    return (uint8_t)(flags & ~(1u << bit));
}

int ot_codec_format_float(char *buf, size_t cap, float v, bool valid)
{
    if (!buf || cap == 0) {
        return -1;
    }
    if (!valid) {
        buf[0] = '\0';
        return 0;
    }
    return snprintf(buf, cap, "%.2f", (double)v);
}

int ot_codec_format_u16(char *buf, size_t cap, uint16_t v, bool valid)
{
    if (!buf || cap == 0) {
        return -1;
    }
    if (!valid) {
        buf[0] = '\0';
        return 0;
    }
    return snprintf(buf, cap, "%u", (unsigned)v);
}

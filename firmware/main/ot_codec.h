#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** OpenTherm f8.8 ↔ float (°C). */
float ot_codec_f88_to_float(uint16_t raw);
uint16_t ot_codec_float_to_f88(float celsius);

/** Pack/unpack high/low bytes of DATA-VALUE. */
uint16_t ot_codec_pack_hb_lb(uint8_t hb, uint8_t lb);
void ot_codec_unpack_hb_lb(uint16_t raw, uint8_t *hb, uint8_t *lb);

/** flag8 bit helpers. */
bool ot_codec_flag8_get(uint8_t flags, unsigned bit);
uint8_t ot_codec_flag8_set(uint8_t flags, unsigned bit, bool value);

/** Format numeric state for MQTT (empty string if invalid). */
int ot_codec_format_float(char *buf, size_t cap, float v, bool valid);
int ot_codec_format_u16(char *buf, size_t cap, uint16_t v, bool valid);

#ifdef __cplusplus
}
#endif

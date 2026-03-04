#ifndef FONT_H
#define FONT_H

#include <stdint.h>

extern const uint8_t font8x8_basic[128][8];

void draw_char(uint32_t x, uint32_t y, unsigned char c, uint32_t fg, uint32_t bg);
void draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg_color, uint32_t bg_color);

#endif

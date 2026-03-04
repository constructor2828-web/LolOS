#ifndef GUI_H
#define GUI_H

#include <stdint.h>

void draw_desktop(void);
void draw_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const char* title);

#endif

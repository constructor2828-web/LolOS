#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

extern int32_t mouse_x;
extern int32_t mouse_y;
extern uint8_t mouse_buttons;

void init_mouse(void);

#endif

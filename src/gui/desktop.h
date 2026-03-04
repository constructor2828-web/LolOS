#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>

typedef enum {
    OS_STATE_LOGIN   = 0,
    OS_STATE_DESKTOP = 1
} os_state_t;

extern os_state_t os_state;

// App window IDs
#define APP_TERMINAL   0
#define APP_FILEMANAGER 1
#define APP_CALCULATOR 2
#define APP_TEXTEDITOR 3
#define APP_BROWSER    4
#define APP_COUNT      5

extern int focused_app;   // which app has keyboard focus

void desktop_init(void);
void desktop_draw(void);
void desktop_on_click(int32_t x, int32_t y);
void desktop_on_mouse_move(int32_t x, int32_t y);
void desktop_on_mouse_button(int32_t x, int32_t y, uint8_t buttons);
void desktop_keyboard(char c);

#endif

#include "gui.h"
#include "graphics.h"
#include "font.h"

// Desktop colors
#define COLOR_DESKTOP_BG  0x008080 // Teal desktop
#define COLOR_TASKBAR_BG  0xC0C0C0 // Classic gray
#define COLOR_WINDOW_BG   0xC0C0C0
#define COLOR_TITLEBAR_BG 0x000080
#define COLOR_TEXT_WHITE  0xFFFFFF
#define COLOR_TEXT_BLACK  0x000000
#define COLOR_FRAME_LIGHT 0xFFFFFF
#define COLOR_FRAME_DARK  0x808080
#define COLOR_FRAME_BLACK 0x000000

void draw_desktop(void) {
    // Fill desktop background
    fill_rect(0, 0, current_width, current_height - 30, COLOR_DESKTOP_BG);
    
    // Draw taskbar at the bottom
    fill_rect(0, current_height - 30, current_width, 30, COLOR_TASKBAR_BG);
    draw_rect(0, current_height - 30, current_width, 30, COLOR_FRAME_LIGHT); // Top border of taskbar

    // Draw a "Start" button
    fill_rect(2, current_height - 28, 60, 26, COLOR_TASKBAR_BG);
    draw_rect(2, current_height - 28, 60, 26, COLOR_FRAME_LIGHT); // Make it look raised
    draw_rect(3, current_height - 27, 58, 24, COLOR_FRAME_DARK);

    draw_string(14, current_height - 20, "Start", COLOR_TEXT_BLACK, 0);
}

void draw_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const char* title) {
    // Main window background
    fill_rect(x, y, width, height, COLOR_WINDOW_BG);

    // Light border (top and left)
    fill_rect(x, y, width, 2, COLOR_FRAME_LIGHT);
    fill_rect(x, y, 2, height, COLOR_FRAME_LIGHT);

    // Dark border (bottom and right)
    fill_rect(x, y + height - 2, width, 2, COLOR_FRAME_BLACK);
    fill_rect(x + width - 2, y, 2, height, COLOR_FRAME_BLACK);
    fill_rect(x + 1, y + height - 3, width - 2, 1, COLOR_FRAME_DARK);
    fill_rect(x + width - 3, y + 1, 1, height - 2, COLOR_FRAME_DARK);

    // Titlebar
    uint32_t titlebar_height = 20;
    fill_rect(x + 3, y + 3, width - 6, titlebar_height, COLOR_TITLEBAR_BG);
    
    // Title text
    draw_string(x + 6, y + 9, title, COLOR_TEXT_WHITE, 0);
}

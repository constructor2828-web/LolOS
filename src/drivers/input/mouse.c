#include "mouse.h"
#include "isr.h"
#include "io.h"
#include "graphics.h"
#include <stdint.h>

int32_t mouse_x = 512;
int32_t mouse_y = 384;
uint8_t mouse_buttons = 0;

// Previous cursor position to erase before redrawing
static int32_t prev_x = 512;
static int32_t prev_y = 384;

#define CURSOR_COLOR 0xFFFFFF
#define CURSOR_SHADOW 0x000000

// 8x8 arrow cursor bitmap (1 = fill, 2 = outline, 0 = transparent)
static const uint8_t cursor_shape[8][8] = {
    {1, 2, 0, 0, 0, 0, 0, 0},
    {1, 1, 2, 0, 0, 0, 0, 0},
    {1, 1, 1, 2, 0, 0, 0, 0},
    {1, 1, 1, 1, 2, 0, 0, 0},
    {1, 1, 1, 1, 1, 2, 0, 0},
    {1, 1, 2, 2, 2, 0, 0, 0},
    {1, 2, 0, 1, 2, 0, 0, 0},
    {2, 0, 0, 0, 1, 2, 0, 0},
};

// Saved pixels under cursor for restoration
static uint32_t saved_bg[8][8];

static void save_cursor_bg(int32_t x, int32_t y) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int32_t px = x + col, py = y + row;
            if (px >= 0 && px < (int32_t)current_width && py >= 0 && py < (int32_t)current_height) {
                saved_bg[row][col] = current_framebuffer[py * (current_pitch / 4) + px];
            }
        }
    }
}

static void restore_cursor_bg(int32_t x, int32_t y) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int32_t px = x + col, py = y + row;
            if (px >= 0 && px < (int32_t)current_width && py >= 0 && py < (int32_t)current_height) {
                current_framebuffer[py * (current_pitch / 4) + px] = saved_bg[row][col];
            }
        }
    }
}

static void draw_cursor(int32_t x, int32_t y) {
    save_cursor_bg(x, y);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int32_t px = x + col, py = y + row;
            if (px >= 0 && px < (int32_t)current_width && py >= 0 && py < (int32_t)current_height) {
                uint8_t p = cursor_shape[row][col];
                if (p == 1) put_pixel(px, py, CURSOR_COLOR);
                else if (p == 2) put_pixel(px, py, CURSOR_SHADOW);
            }
        }
    }
}

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if (inb(0x64) & 0x01) return;
        }
    } else {
        while (timeout--) {
            if (!(inb(0x64) & 0x02)) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

static void mouse_callback(registers_t* regs) {
    (void)regs;
    uint8_t status = inb(0x64);
    if (!(status & 0x01)) return;  // no data available

    uint8_t data = inb(0x60);

    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = data;
            // bit 3 must be set on the first byte
            if (!(data & 0x08)) break;
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;

            // Parse mouse movement
            int32_t dx = (int32_t)(int8_t)mouse_byte[1];
            int32_t dy = (int32_t)(int8_t)mouse_byte[2];

            // Erase old cursor
            restore_cursor_bg(prev_x, prev_y);

            mouse_x += dx;
            mouse_y -= dy;   // Y is inverted in screen coords

            // Clamp
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= (int32_t)current_width - 1) mouse_x = (int32_t)current_width - 1;
            if (mouse_y >= (int32_t)current_height - 1) mouse_y = (int32_t)current_height - 1;

            uint8_t buttons = mouse_byte[0] & 0x07;
            
            extern void desktop_on_mouse_move(int32_t x, int32_t y);
            extern void desktop_on_mouse_button(int32_t x, int32_t y, uint8_t buttons);

            if (dx != 0 || dy != 0) {
                desktop_on_mouse_move(mouse_x, mouse_y);
            }

            if (buttons != mouse_buttons) {
                desktop_on_mouse_button(mouse_x, mouse_y, buttons);
            }

            mouse_buttons = buttons;

            // Draw cursor at new position
            draw_cursor(mouse_x, mouse_y);
            
            // Partial swap: erase old and draw new
            swap_buffers_rect(prev_x, prev_y, 8, 8);
            swap_buffers_rect(mouse_x, mouse_y, 8, 8);
            
            prev_x = mouse_x;
            prev_y = mouse_y;
            break;
    }
}

void init_mouse(void) {
    uint8_t status;

    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // Use default settings
    mouse_write(0xF6);
    mouse_read(); // acknowledge

    // Enable the mouse
    mouse_write(0xF4);
    mouse_read(); // acknowledge

    // IRQ12 is at vector 44
    register_interrupt_handler(44, mouse_callback);

    // Draw initial cursor
    draw_cursor(mouse_x, mouse_y);
    prev_x = mouse_x;
    prev_y = mouse_y;
}

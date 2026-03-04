#include "graphics.h"
#include "memory.h"
#include <stddef.h>

static uint32_t* front_buffer = NULL;
static uint32_t* back_buffer = NULL;

uint32_t* current_framebuffer = NULL; // This will point to back_buffer
uint32_t current_width = 0;
uint32_t current_height = 0;
uint32_t current_pitch = 0;

void init_graphics(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch) {
    front_buffer = framebuffer;
    current_width = width;
    current_height = height;
    current_pitch = pitch;

    // Allocate back buffer using our new kmalloc
    // Pitch / 4 is the number of 32-bit words per line
    size_t buffer_size = (pitch / 4) * height * sizeof(uint32_t);
    back_buffer = (uint32_t*)kmalloc_aligned(buffer_size, 4096);
    
    // Set current draw target to the back buffer
    current_framebuffer = back_buffer;
}

void swap_buffers(void) {
    swap_buffers_rect(0, 0, current_width, current_height);
}

void swap_buffers_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (!front_buffer || !back_buffer) return;
    if (x >= current_width || y >= current_height) return;
    if (x + w > current_width) w = current_width - x;
    if (y + h > current_height) h = current_height - y;

    uint32_t pitch_words = current_pitch / 4;
    for (uint32_t row = 0; row < h; row++) {
        uint32_t* src_line = &back_buffer[(y + row) * pitch_words + x];
        uint32_t* dst_line = &front_buffer[(y + row) * pitch_words + x];
        for (uint32_t i = 0; i < w; i++) {
            dst_line[i] = src_line[i];
        }
    }
}

void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!current_framebuffer) return;
    if (x >= current_width || y >= current_height) return;

    uint32_t location = x + (y * (current_pitch / 4));
    current_framebuffer[location] = color;
}

void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t i = 0; i < width; i++) {
        put_pixel(x + i, y, color);
        put_pixel(x + i, y + height - 1, color);
    }
    for (uint32_t i = 0; i < height; i++) {
        put_pixel(x, y + i, color);
        put_pixel(x + width - 1, y + i, color);
    }
}

void fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!current_framebuffer) return;
    if (x >= current_width || y >= current_height) return;
    if (x + width > current_width) width = current_width - x;
    if (y + height > current_height) height = current_height - y;

    uint32_t pitch_words = current_pitch / 4;
    for (uint32_t i = 0; i < height; i++) {
        uint32_t* dst_line = &current_framebuffer[(y + i) * pitch_words + x];
        for (uint32_t j = 0; j < width; j++) {
            dst_line[j] = color;
        }
    }
}

void draw_image(int32_t x, int32_t y, uint32_t width, uint32_t height, const uint32_t* data) {
    if (!current_framebuffer || !data) return;
    uint32_t pitch_words = current_pitch / 4;
    for (uint32_t row = 0; row < height; row++) {
        int32_t py = y + (int32_t)row;
        if (py < 0 || py >= (int32_t)current_height) continue;
        uint32_t* line = &current_framebuffer[py * pitch_words];
        for (uint32_t col = 0; col < width; col++) {
            int32_t px = x + (int32_t)col;
            if (px < 0 || px >= (int32_t)current_width) continue;
            uint32_t color = data[row * width + col];
            if (color != 0xFF00FF) {
                line[px] = color;
            }
        }
    }
}

void clear_screen(uint32_t color) {
    if (!current_framebuffer) return;
    uint32_t total_words = (current_pitch / 4) * current_height;
    for (uint32_t i = 0; i < total_words; i++) {
        current_framebuffer[i] = color;
    }
}

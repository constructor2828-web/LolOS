#include "login.h"
#include "graphics.h"
#include "font.h"
#include "desktop.h"
#include <stdint.h>

#define DLG_W    380
#define DLG_H    200
#define PASS_MAX 32

static char pass_buf[PASS_MAX + 1];
static int  pass_len = 0;
static int  shake_timer = 0;  // >0 = show error message

static const char* CORRECT_PASSWORD = "admin1234";

static int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

void login_init(void) {
    pass_len = 0;
    pass_buf[0] = '\0';
    shake_timer = 0;
}

void login_draw(void) {
    uint32_t sw = current_width, sh = current_height;

    // Draw dark blurred background
    fill_rect(0, 0, sw, sh, 0x1A2340);

    // Draw grid pattern for style
    for (uint32_t y = 0; y < sh; y += 32) {
        fill_rect(0, y, sw, 1, 0x1E2B50);
    }
    for (uint32_t x = 0; x < sw; x += 32) {
        fill_rect(x, 0, 1, sh, 0x1E2B50);
    }

    // Draw OS name at top
    draw_string(sw/2 - 28, sh/4, "LoLOS", 0x5BA4F5, 0);
    draw_string(sw/2 - 56, sh/4 + 12, "Version 1.0", 0x778899, 0);

    // Center the dialog
    uint32_t dx = (sw - DLG_W) / 2;
    uint32_t dy = (sh - DLG_H) / 2;

    // Dialog shadow
    fill_rect(dx + 4, dy + 4, DLG_W, DLG_H, 0x000000);

    // Dialog background
    fill_rect(dx, dy, DLG_W, DLG_H, 0xEEEEEE);

    // Blue title bar
    fill_rect(dx, dy, DLG_W, 28, 0x1C5FA5);
    draw_string(dx + 8, dy + 10, "LoLOS - Login", 0xFFFFFF, 0);

    // Inner content area
    fill_rect(dx + 12, dy + 36, DLG_W - 24, DLG_H - 48, 0xDDDDDD);

    // Username row
    draw_string(dx + 20, dy + 50, "Username:", 0x222222, 0xDDDDDD);
    fill_rect(dx + 110, dy + 46, 220, 18, 0xFFFFFF);
    draw_rect(dx + 110, dy + 46, 220, 18, 0x888888);
    draw_string(dx + 114, dy + 50, "admin", 0x000000, 0xFFFFFF);

    // Password row
    draw_string(dx + 20, dy + 80, "Password:", 0x222222, 0xDDDDDD);
    fill_rect(dx + 110, dy + 76, 220, 18, 0xFFFFFF);
    draw_rect(dx + 110, dy + 76, 220, 18, shake_timer > 0 ? 0xFF0000 : 0x888888);

    // Draw masked password
    for (int i = 0; i < pass_len; i++) {
        fill_rect(dx + 114 + i * 8, dy + 81, 6, 8, 0x000000);
    }

    // Blinking cursor in password field
    fill_rect(dx + 114 + pass_len * 8, dy + 79, 2, 12, 0x0044CC);

    // Error message
    if (shake_timer > 0) {
        draw_string(dx + 20, dy + 108, "Incorrect password. Try again.", 0xCC0000, 0xDDDDDD);
        shake_timer--;
    } else {
        draw_string(dx + 20, dy + 108, "Enter your password and press Enter.", 0x555555, 0xDDDDDD);
    }

    // Login button
    uint32_t btn_x = dx + DLG_W - 100;
    uint32_t btn_y = dy + DLG_H - 38;
    fill_rect(btn_x, btn_y, 80, 24, 0x1C5FA5);
    draw_rect(btn_x, btn_y, 80, 24, 0x0A3D7A);
    draw_string(btn_x + 22, btn_y + 8, "Login", 0xFFFFFF, 0);
}

void login_key(char c) {
    if (c == '\n') {
        // Check password
        pass_buf[pass_len] = '\0';
        if (k_strcmp(pass_buf, CORRECT_PASSWORD) == 0) {
            os_state = OS_STATE_DESKTOP;
            desktop_draw();
        } else {
            shake_timer = 60;
            pass_len = 0;
            login_draw();
        }
    } else if (c == '\b') {
        if (pass_len > 0) pass_len--;
        login_draw();
    } else if (pass_len < PASS_MAX) {
        pass_buf[pass_len++] = c;
        login_draw();
    }
    swap_buffers();
}

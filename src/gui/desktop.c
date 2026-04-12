#include "desktop.h"
#include "login.h"
#include "apps.h"
#include "graphics.h"
#include "font.h"
#include "gui.h"
#include "vfs.h"
#include "icons.h"
#include <stdint.h>

os_state_t os_state   = OS_STATE_LOGIN;
int        focused_app = APP_TERMINAL;
static int is_dragging = 0;
static int dragged_app_id = -1;
static int drag_off_x = 0;
static int drag_off_y = 0;

/* -------------------------------------------------------
 * Taskbar layout (bottom 32px)
 * -------------------------------------------------------*/
#define TASKBAR_H 32
#define TB_BTN_W  90
#define TB_BTN_H  24
#define TB_BTN_PAD 4

static const char* tb_labels[APP_COUNT] = {
    "Terminal", "Files", "Calculator", "Editor", "Browser"
};

static void draw_taskbar(void) {
    uint32_t sw = current_width, sh = current_height;
    uint32_t ty = sh - TASKBAR_H;

    // Bar background with gradient look
    fill_rect(0, ty, sw, TASKBAR_H, 0x2B3A55);
    fill_rect(0, ty, sw, 1, 0x4A6494);      // top highlight line

    // Start/Logo button
    fill_rect(2, ty + 4, 60, 24, 0x1C5FA5);
    draw_rect(2, ty + 4, 60, 24, 0x5588CC);
    draw_string(8, ty + 12, "LoLOS", 0xFFFFFF, 0);

    // App buttons
    for (int i = 0; i < APP_COUNT; i++) {
        uint32_t bx = 70 + i * (TB_BTN_W + TB_BTN_PAD);
        uint32_t by = ty + 4;

        // Highlight if app is visible
        uint8_t visible = 0;
        if (i == APP_TERMINAL)    visible = g_terminal.visible;
        if (i == APP_FILEMANAGER) visible = g_filemanager.visible;
        if (i == APP_CALCULATOR)  visible = g_calculator.visible;
        if (i == APP_TEXTEDITOR)  visible = g_texteditor.visible;
        if (i == APP_BROWSER)     visible = g_browser.visible;

        uint32_t bg = visible ? 0x4A6EB0 : 0x334466;
        uint32_t border = (i == focused_app) ? 0xFFAA00 : 0x556688;

        fill_rect(bx, by, TB_BTN_W, TB_BTN_H, bg);
        draw_rect(bx, by, TB_BTN_W, TB_BTN_H, border);
        
        const uint32_t* icons[] = {icon_terminal, icon_files, icon_calc, icon_editor, icon_browser};
        draw_image(bx + 4, by + 4, 16, 16, icons[i]);
        draw_string(bx + 24, by + 8, tb_labels[i], 0xFFFFFF, bg);
    }

    // Clock area (static for now)
    draw_string(sw - 56, ty + 12, "00:00", 0xCCDDFF, 0);
}

void desktop_draw(void) {
    // Background / wallpaper
    fill_rect(0, 0, current_width, current_height - TASKBAR_H, 0x1A3A5C);

    // Simple grid pattern wallpaper
    for (uint32_t y = 0; y < current_height - TASKBAR_H; y += 40)
        fill_rect(0, y, current_width, 1, 0x204060);
    for (uint32_t x = 0; x < current_width; x += 40)
        fill_rect(x, 0, 1, current_height - TASKBAR_H, 0x204060);

    // OS name watermark
    draw_string(current_width - 56, current_height - TASKBAR_H - 20, "LoLOS", 0x2A4A70, 0);

    // Desktop icons (top-left column)
    uint32_t ic_y = 20;
    const uint32_t* icons[] = {icon_terminal, icon_files, icon_calc, icon_editor, icon_browser};
    for (int i = 0; i < APP_COUNT; i++) {
        // Transparent background for icons
        draw_image(20, ic_y, 16, 16, icons[i]);
        draw_string(8, ic_y + 20, tb_labels[i], 0xCCDDFF, 0);
        ic_y += 50;
    }

    // Redraw visible windows
    if (g_terminal.visible)    terminal_draw();
    if (g_filemanager.visible) filemanager_draw();
    if (g_calculator.visible)  calculator_draw();
    if (g_texteditor.visible)  texteditor_draw();
    if (g_browser.visible)     browser_draw();

    // Taskbar on top (always)
    draw_taskbar();
}

/* -------------------------------------------------------
 * Click hit test helpers
 * -------------------------------------------------------*/
static int hit(int32_t cx, int32_t cy, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    return cx >= (int32_t)x && cx < (int32_t)(x+w) && cy >= (int32_t)y && cy < (int32_t)(y+h);
}

static void toggle_app(int app_id) {
    switch (app_id) {
        case APP_TERMINAL:    g_terminal.visible    ^= 1; terminal_draw();    break;
        case APP_FILEMANAGER: g_filemanager.visible ^= 1; filemanager_draw(); break;
        case APP_CALCULATOR:  g_calculator.visible  ^= 1; calculator_draw();  break;
        case APP_TEXTEDITOR:  g_texteditor.visible  ^= 1; texteditor_draw();  break;
        case APP_BROWSER:     g_browser.visible     ^= 1; browser_draw();     break;
    }
    focused_app = app_id;
    draw_taskbar();
}

void desktop_on_click(int32_t x, int32_t y) {
    if (os_state != OS_STATE_DESKTOP) return;
    uint32_t ty = current_height - TASKBAR_H;

    /* ---- Taskbar buttons ---- */
    if (y >= (int32_t)ty) {
        for (int i = 0; i < APP_COUNT; i++) {
            int32_t bx = (int32_t)(70 + i * (TB_BTN_W + TB_BTN_PAD));
            int32_t by = (int32_t)(ty + 4);
            if (hit(x, y, bx, by, TB_BTN_W, TB_BTN_H)) {
                toggle_app(i);
                return;
            }
        }
        return;
    }

    /* ---- Desktop icons ---- */
    int32_t ic_y = 20;
    for (int i = 0; i < APP_COUNT; i++) {
        if (hit(x, y, 8, ic_y, 64, 48)) {
            toggle_app(i);
            return;
        }
        ic_y += 50; // Match desktop_draw spacing
    }

    /* ---- Terminal window ---- */
    if (g_terminal.visible) {
        uint32_t wx = g_terminal.x, wy = g_terminal.y;
        uint32_t ww = g_terminal.width;
        // Close button
        if (hit(x, y, wx + ww - 20, wy + 4, 16, 16)) {
            g_terminal.visible = 0;
            desktop_draw(); return;
        }
        // Focus
        if (hit(x, y, wx, wy, ww, g_terminal.height)) {
            focused_app = APP_TERMINAL; draw_taskbar(); return;
        }
    }

    /* ---- File Manager window ---- */
    if (g_filemanager.visible) {
        uint32_t wx = g_filemanager.x, wy = g_filemanager.y;
        uint32_t ww = g_filemanager.width, wh = g_filemanager.height;
        if (hit(x, y, wx + ww - 20, wy + 4, 16, 16)) {
            g_filemanager.visible = 0; desktop_draw(); return;
        }
        // Row selection
        if (hit(x, y, wx, wy + 44, ww, wh - 64)) {
            int rel_y = (int)(y - (int32_t)(wy + 44));
            g_filemanager.selected = rel_y / 18;
            focused_app = APP_FILEMANAGER;
            filemanager_draw(); draw_taskbar(); return;
        }
        if (hit(x, y, wx, wy, ww, wh)) { focused_app = APP_FILEMANAGER; draw_taskbar(); return; }
    }

    /* ---- Calculator window ---- */
    if (g_calculator.visible) {
        uint32_t wx = g_calculator.x, wy = g_calculator.y;
        uint32_t ww = g_calculator.width;
        if (hit(x, y, wx + ww - 20, wy + 4, 16, 16)) {
            g_calculator.visible = 0; desktop_draw(); return;
        }
        // Button clicks
        calculator_click(x, y);
        focused_app = APP_CALCULATOR; draw_taskbar();
    }

    /* ---- Text Editor ---- */
    if (g_texteditor.visible) {
        uint32_t wx = g_texteditor.x, wy = g_texteditor.y;
        uint32_t ww = g_texteditor.width, wh = g_texteditor.height;
        if (hit(x, y, wx + ww - 20, wy + 4, 16, 16)) {
            g_texteditor.visible = 0; desktop_draw(); return;
        }
        // Save button
        if (hit(x, y, wx + 6, wy + 29, 48, 14)) {
            g_texteditor.buf[g_texteditor.buf_len] = '\0';
            vfs_create(g_texteditor.filename, g_texteditor.buf);
            return;
        }
        if (hit(x, y, wx, wy, ww, wh)) { focused_app = APP_TEXTEDITOR; draw_taskbar(); return; }
    }

    /* ---- Browser window ---- */
    if (g_browser.visible) {
        uint32_t wx = g_browser.x, wy = g_browser.y;
        uint32_t ww = g_browser.width, wh = g_browser.height;
        if (hit(x, y, wx + ww - 20, wy + 4, 16, 16)) {
            g_browser.visible = 0; desktop_draw(); return;
        }
        if (hit(x, y, wx, wy, ww, wh)) { focused_app = APP_BROWSER; draw_taskbar(); return; }
    }
}

void desktop_on_mouse_button(int32_t x, int32_t y, uint8_t buttons) {
    if (os_state != OS_STATE_DESKTOP) return;

    static uint8_t prev_buttons = 0;
    uint8_t pressed = (buttons & 1) && !(prev_buttons & 1);
    uint8_t released = !(buttons & 1) && (prev_buttons & 1);
    prev_buttons = buttons;

    if (pressed) {
        // Check for dragging (title bars)
        // Check windows in reverse order (topmost first)
        struct { uint8_t visible; uint32_t x, y, w, h; int id; } wins[] = {
            {g_texteditor.visible,  g_texteditor.x,  g_texteditor.y,  g_texteditor.width,  24, APP_TEXTEDITOR},
            {g_browser.visible,     g_browser.x,     g_browser.y,     g_browser.width,     24, APP_BROWSER},
            {g_calculator.visible,  g_calculator.x,  g_calculator.y,  g_calculator.width,  24, APP_CALCULATOR},
            {g_filemanager.visible, g_filemanager.x, g_filemanager.y, g_filemanager.width, 24, APP_FILEMANAGER},
            {g_terminal.visible,    g_terminal.x,    g_terminal.y,    g_terminal.width,    24, APP_TERMINAL}
        };

        for (int i = 0; i < 5; i++) {
            if (wins[i].visible && hit(x, y, wins[i].x, wins[i].y, wins[i].w, wins[i].h)) {
                // If it's a close button, desktop_on_click will handle it
                if (hit(x, y, wins[i].x + wins[i].w - 20, wins[i].y + 4, 16, 16)) {
                    desktop_on_click(x, y);
                    return;
                }
                is_dragging = 1;
                dragged_app_id = wins[i].id;
                drag_off_x = x - (int32_t)wins[i].x;
                drag_off_y = y - (int32_t)wins[i].y;
                focused_app = wins[i].id;
                draw_taskbar();
                return;
            }
        }
        // If not dragging, handle normal click
        desktop_on_click(x, y);
    } else if (released) {
        is_dragging = 0;
        dragged_app_id = -1;
    }
}

void desktop_on_mouse_move(int32_t x, int32_t y) {
    if (os_state != OS_STATE_DESKTOP) return;
    if (!is_dragging) return;

    // Save old box for partial swap
    uint32_t old_x = 0, old_y = 0, w = 0, h = 0;
    switch (dragged_app_id) {
        case APP_TERMINAL:    old_x = g_terminal.x;    old_y = g_terminal.y;    w = g_terminal.width;  h = g_terminal.height; break;
        case APP_FILEMANAGER: old_x = g_filemanager.x; old_y = g_filemanager.y; w = g_filemanager.width; h = g_filemanager.height; break;
        case APP_CALCULATOR:  old_x = g_calculator.x;  old_y = g_calculator.y;  w = g_calculator.width; h = g_calculator.height; break;
        case APP_TEXTEDITOR:  old_x = g_texteditor.x;  old_y = g_texteditor.y;  w = g_texteditor.width; h = g_texteditor.height; break;
        case APP_BROWSER:     old_x = g_browser.x;     old_y = g_browser.y;     w = g_browser.width;    h = g_browser.height; break;
    }

    int32_t unclamped_x = x - drag_off_x;
    int32_t unclamped_y = y - drag_off_y;
    int32_t max_x = (int32_t)current_width - (int32_t)w;
    int32_t max_y = (int32_t)(current_height - TASKBAR_H) - (int32_t)h;

    if (max_x < 0) max_x = 0;
    if (max_y < 0) max_y = 0;
    if (unclamped_x < 0) unclamped_x = 0;
    if (unclamped_y < 0) unclamped_y = 0;
    if (unclamped_x > max_x) unclamped_x = max_x;
    if (unclamped_y > max_y) unclamped_y = max_y;

    uint32_t new_x = (uint32_t)unclamped_x;
    uint32_t new_y = (uint32_t)unclamped_y;

    switch (dragged_app_id) {
        case APP_TERMINAL:    g_terminal.x = new_x; g_terminal.y = new_y; break;
        case APP_FILEMANAGER: g_filemanager.x = new_x; g_filemanager.y = new_y; break;
        case APP_CALCULATOR:  g_calculator.x = new_x; g_calculator.y = new_y; break;
        case APP_TEXTEDITOR:  g_texteditor.x = new_x; g_texteditor.y = new_y; break;
        case APP_BROWSER:     g_browser.x = new_x; g_browser.y = new_y; break;
    }

    desktop_draw();
    
    // Swap old hole and new box
    swap_buffers_rect(old_x, old_y, w, h);
    swap_buffers_rect(new_x, new_y, w, h);
}

void desktop_keyboard(char c) {
    if (os_state == OS_STATE_LOGIN) {
        login_key(c);
        return;
    }
    // Route to focused app
    switch (focused_app) {
        case APP_TERMINAL:   if (g_terminal.visible)    terminal_type(c);   break;
        case APP_TEXTEDITOR: if (g_texteditor.visible)  texteditor_type(c); break;
        default: break;
    }
}

void desktop_init(void) {
    os_state   = OS_STATE_LOGIN;
    focused_app = APP_TERMINAL;

    terminal_init(200, 80, 500, 300);
    filemanager_init(730, 80, 280, 400);
    calculator_init(260, 140, 196, 210);
    texteditor_init(240, 120, 520, 320);
    browser_init(150, 150, 400, 300);
}

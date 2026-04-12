#include "apps.h"
#include "graphics.h"
#include "font.h"
#include "vfs.h"
#include "io.h"
#include <stdint.h>

terminal_t    g_terminal;
filemanager_t g_filemanager;
calculator_t  g_calculator;
texteditor_t  g_texteditor;
browser_t     g_browser;

/* ============================================================
 * Shared helpers
 * ============================================================ */

static void draw_titlebar(uint32_t x, uint32_t y, uint32_t w, const char* title, uint32_t color) {
    uint32_t dark = ((color >> 1) & 0x7F7F7F);
    uint32_t light = color | 0x202020;

    // Soft vertical gradient
    for (uint32_t row = 0; row < 24; row++) {
        uint32_t c = (row < 12) ? light : dark;
        fill_rect(x, y + row, w, 1, c);
    }

    // Beveled titlebar frame
    fill_rect(x, y, w, 1, 0xFFFFFF);
    fill_rect(x, y, 1, 24, 0xFFFFFF);
    fill_rect(x, y + 23, w, 1, 0x1F1F1F);
    fill_rect(x + w - 1, y, 1, 24, 0x1F1F1F);

    draw_string(x + 8, y + 8, title, 0xFFFFFF, 0);

    // Close (X) button with raised style
    uint32_t bx = x + w - 22;
    uint32_t by = y + 4;
    fill_rect(bx, by, 16, 16, 0xB03030);
    fill_rect(bx, by, 16, 1, 0xF07A7A);
    fill_rect(bx, by, 1, 16, 0xF07A7A);
    fill_rect(bx, by + 15, 16, 1, 0x6A1111);
    fill_rect(bx + 15, by, 1, 16, 0x6A1111);
    draw_string(bx + 4, by + 4, "X", 0xFFFFFF, 0);
}

static void draw_shadow_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t bg) {
    // Layered shadow for more depth
    fill_rect(x + 2, y + 2, w, h, 0x202020);
    fill_rect(x + 4, y + 4, w, h, 0x111111);
    fill_rect(x, y, w, h, bg);
    // Classic 3D frame
    fill_rect(x, y, w, 2, 0xFFFFFF);
    fill_rect(x, y, 2, h, 0xFFFFFF);
    fill_rect(x, y + h - 2, w, 2, 0x3A3A3A);
    fill_rect(x + w - 2, y, 2, h, 0x3A3A3A);
    fill_rect(x + 1, y + h - 3, w - 2, 1, 0x7A7A7A);
    fill_rect(x + w - 3, y + 1, 1, h - 2, 0x7A7A7A);
}

/* ============================================================
 * Terminal
 * ============================================================ */

#define TERM_BG  0x0C0C18
#define TERM_FG  0x00FF88

void terminal_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    g_terminal.x       = x;
    g_terminal.y       = y;
    g_terminal.width   = w;
    g_terminal.height  = h;
    g_terminal.buf_len = 0;
    g_terminal.visible = 1;
    const char* welcome = "LoLOS Terminal v1.0\n> ";
    for (int i = 0; welcome[i]; i++)
        g_terminal.buf[g_terminal.buf_len++] = welcome[i];
}

void terminal_draw(void) {
    if (!g_terminal.visible) return;
    uint32_t x = g_terminal.x, y = g_terminal.y;
    uint32_t w = g_terminal.width, h = g_terminal.height;
    
    // Calculate how many lines we have and scroll once if needed
    // Simple iterative scroll to fit
    while (1) {
        uint32_t test_cx = x + 6, test_cy = y + 30;
        int needs_scroll = 0;
        for (int i = 0; i < g_terminal.buf_len; i++) {
            char c = g_terminal.buf[i];
            if (c == '\n') { test_cx = x + 6; test_cy += 10; }
            else { test_cx += 8; }
            if (test_cx + 8 > x + w - 6) { test_cx = x + 6; test_cy += 10; }
            if (test_cy + 10 > y + h - 6) { needs_scroll = 1; break; }
        }
        if (!needs_scroll) break;
        
        // Remove first line
        int skip = 0;
        while (skip < (int)g_terminal.buf_len && g_terminal.buf[skip] != '\n') skip++;
        if (skip < (int)g_terminal.buf_len) skip++; // Skip the newline too
        else if (skip == 0 && g_terminal.buf_len > 0) skip = 1;

        if (skip > 0 && skip <= (int)g_terminal.buf_len) {
            for (int k = 0; k < (int)g_terminal.buf_len - skip; k++)
                g_terminal.buf[k] = g_terminal.buf[k + skip];
            g_terminal.buf_len -= skip;
        } else {
            g_terminal.buf_len = 0;
            break;
        }
    }

    // Now draw for real
    draw_shadow_box(x, y, w, h, TERM_BG);
    draw_titlebar(x, y, w, "Terminal", 0x1C1C6E);
    uint32_t cx = x + 6, cy = y + 30;
    for (int i = 0; i < g_terminal.buf_len; i++) {
        char c = g_terminal.buf[i];
        if (c == '\n') { cx = x + 6; cy += 10; }
        else { draw_char(cx, cy, c, TERM_FG, TERM_BG); cx += 8; }
        if (cx + 8 > x + w - 6) { cx = x + 6; cy += 10; }
    }
    fill_rect(cx, cy, 6, 9, TERM_FG); // cursor
}

static void terminal_write_str(const char* s) {
    for (int i = 0; s[i]; i++) {
        if (g_terminal.buf_len < 2047) {
            g_terminal.buf[g_terminal.buf_len++] = s[i];
        }
    }
}

static void terminal_execute_command(char* cmd) {
    if (cmd[0] == '\0') return;

    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p') {
        terminal_write_str("Commands: help, clear, ls, whoami, reboot, echo\n");
    } else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r') {
        g_terminal.buf_len = 0;
    } else if (cmd[0] == 'l' && cmd[1] == 's') {
        int cnt = vfs_count();
        for (int i = 0; i < cnt; i++) {
            vfs_file_t* f = vfs_get(i);
            if (f) {
                terminal_write_str(f->name);
                terminal_write_str("  ");
            }
        }
        terminal_write_str("\n");
    } else if (cmd[0] == 'w' && cmd[1] == 'h' && cmd[2] == 'o' && cmd[3] == 'a' && cmd[4] == 'm' && cmd[5] == 'i') {
        terminal_write_str("admin\n");
    } else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'o' && cmd[4] == 'o' && cmd[5] == 't') {
        outb(0x64, 0xFE);
    } else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ') {
        terminal_write_str(cmd + 5);
        terminal_write_str("\n");
    } else {
        terminal_write_str("Unknown command: ");
        terminal_write_str(cmd);
        terminal_write_str("\n");
    }
}

void terminal_type(char c) {
    if (c == '\b') {
        if (g_terminal.buf_len > 0 && g_terminal.buf[g_terminal.buf_len-1] != ' ' && g_terminal.buf[g_terminal.buf_len-1] != '>')
            g_terminal.buf_len--;
    } else if (c == '\n') {
        // Extract command: find last '> '
        char cmd[64];
        int cmd_ptr = 0;
        int last_prompt = -1;
        for (int i = 0; i < g_terminal.buf_len - 1; i++) {
            if (g_terminal.buf[i] == '>' && g_terminal.buf[i+1] == ' ') {
                last_prompt = i + 2;
            }
        }
        
        if (last_prompt != -1) {
            for (int i = last_prompt; i < g_terminal.buf_len && cmd_ptr < 63; i++) {
                cmd[cmd_ptr++] = g_terminal.buf[i];
            }
        }
        cmd[cmd_ptr] = '\0';

        terminal_write_str("\n");
        terminal_execute_command(cmd);
        terminal_write_str("> ");
    } else if (g_terminal.buf_len < 2047) {
        g_terminal.buf[g_terminal.buf_len++] = c;
    }
    terminal_draw();
}

/* ============================================================
 * File Manager
 * ============================================================ */

#define FM_BG  0xD8D4CC
#define FM_ROW 18

void filemanager_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    g_filemanager.x       = x;
    g_filemanager.y       = y;
    g_filemanager.width   = w;
    g_filemanager.height  = h;
    g_filemanager.selected = 0;
    g_filemanager.visible = 1;
}

void filemanager_draw(void) {
    if (!g_filemanager.visible) return;
    uint32_t x = g_filemanager.x, y = g_filemanager.y;
    uint32_t w = g_filemanager.width, h = g_filemanager.height;
    draw_shadow_box(x, y, w, h, FM_BG);
    draw_titlebar(x, y, w, "File Manager", 0x204070);

    // Column header
    fill_rect(x + 2, y + 26, w - 4, FM_ROW, 0xBBB8B0);
    draw_string(x + 6, y + 30, "Name", 0x000000, 0xBBB8B0);
    draw_string(x + w/2, y + 30, "Bytes", 0x000000, 0xBBB8B0);

    int cnt = vfs_count();
    uint32_t ry = y + 26 + FM_ROW;
    for (int i = 0; i < cnt && ry + FM_ROW < y + h - 20; i++) {
        vfs_file_t* f = vfs_get(i);
        if (!f) continue;
        uint32_t rb = (i == g_filemanager.selected) ? 0x2255AA : ((i % 2) ? FM_BG : 0xCAC6BE);
        uint32_t rf = (i == g_filemanager.selected) ? 0xFFFFFF : 0x111111;
        fill_rect(x + 2, ry, w - 4, FM_ROW, rb);
        draw_string(x + 6, ry + 5, f->name, rf, rb);
        // draw size
        char sz[12]; int si = 11; sz[11] = '\0';
        uint32_t s = f->size;
        if (s == 0) { sz[--si] = '0'; } else while (s) { sz[--si] = '0' + s%10; s /= 10; }
        draw_string(x + w/2, ry + 5, &sz[si], rf, rb);
        ry += FM_ROW;
    }
    // Status bar
    fill_rect(x, y + h - 18, w, 18, 0xBBB8B0);
    draw_rect(x, y + h - 18, w, 18, 0x808080);
    draw_string(x + 4, y + h - 13, "LoLOS VFS  - Click to select", 0x222222, 0xBBB8B0);
}

/* ============================================================
 * Calculator
 * ============================================================ */

#define BTN_W 42
#define BTN_H 24
#define BTN_PAD 4

static const char* calc_labels[4][4] = {
    {"7","8","9","/"},
    {"4","5","6","*"},
    {"1","2","3","-"},
    {"0",".","=","+"},
};

static void calc_btn(uint32_t bx, uint32_t by, const char* lbl, uint32_t bg) {
    fill_rect(bx, by, BTN_W, BTN_H, bg);
    draw_rect(bx, by, BTN_W, BTN_H, 0x555555);
    draw_string(bx + BTN_W/2 - 4, by + 8, lbl, 0xFFFFFF, bg);
}

static double simple_atof(const char* s) {
    double v = 0; double frac = 0; int in_frac = 0; double div = 10;
    int neg = 0; if (*s == '-') { neg = 1; s++; }
    while (*s) {
        if (*s == '.') { in_frac = 1; s++; continue; }
        if (*s >= '0' && *s <= '9') {
            if (!in_frac) v = v * 10 + (*s - '0');
            else { frac += (*s - '0') / div; div *= 10; }
        }
        s++;
    }
    return neg ? -(v + frac) : (v + frac);
}

static void int_to_str(char* buf, int n) {
    if (n < 0) { *buf++ = '-'; n = -n; }
    char tmp[12]; int i = 0;
    if (n == 0) { tmp[i++] = '0'; }
    while (n > 0) { tmp[i++] = '0' + n % 10; n /= 10; }
    for (int j = i - 1; j >= 0; j--) *buf++ = tmp[j];
    *buf = '\0';
}

void calculator_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    g_calculator.x          = x;
    g_calculator.y          = y;
    g_calculator.width      = w;
    g_calculator.height     = h;
    g_calculator.display[0] = '0';
    g_calculator.display[1] = '\0';
    g_calculator.operand_a  = 0;
    g_calculator.operand_b  = 0;
    g_calculator.operator   = 0;
    g_calculator.has_operand = 0;
    g_calculator.next_clears = 0;
    g_calculator.visible    = 0; // start hidden, opened from taskbar
}

void calculator_draw(void) {
    if (!g_calculator.visible) return;
    uint32_t x = g_calculator.x, y = g_calculator.y;
    uint32_t w = g_calculator.width;
    draw_shadow_box(x, y, w, g_calculator.height, 0x2A2A2A);
    draw_titlebar(x, y, w, "Calculator", 0x2D6A4F);

    // Display field
    uint32_t disp_y = y + 28;
    fill_rect(x + 6, disp_y, w - 12, 22, 0x111111);
    draw_rect(x + 6, disp_y, w - 12, 22, 0x444444);

    // Right-align display text
    int dlen = 0; while (g_calculator.display[dlen]) dlen++;
    draw_string(x + w - 12 - dlen * 8, disp_y + 7, g_calculator.display, 0x44FF44, 0x111111);

    // Clear button
    fill_rect(x + 6, disp_y + 26, w - 12, 20, 0x884422);
    draw_rect(x + 6, disp_y + 26, w - 12, 20, 0x555555);
    draw_string(x + w/2 - 12, disp_y + 33, "Clear", 0xFFFFFF, 0x884422);

    // Button grid
    uint32_t grid_y = disp_y + 50;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            uint32_t bx = x + 6 + col * (BTN_W + BTN_PAD);
            uint32_t by = grid_y + row * (BTN_H + BTN_PAD);
            const char* lbl = calc_labels[row][col];
            uint32_t bg = (lbl[0] == '=' ) ? 0x1C5FA5 :
                          (lbl[0] == '/' || lbl[0] == '*' ||
                           lbl[0] == '-' || lbl[0] == '+') ? 0x555555 : 0x3A3A3A;
            calc_btn(bx, by, lbl, bg);
        }
    }
}

void calculator_click(int32_t cx, int32_t cy) {
    if (!g_calculator.visible) return;
    uint32_t x = g_calculator.x, y = g_calculator.y, w = g_calculator.width;
    uint32_t disp_y = y + 28;

    // Clear button
    if (cx >= (int32_t)(x+6) && cx < (int32_t)(x+w-6) &&
        cy >= (int32_t)(disp_y+26) && cy < (int32_t)(disp_y+46)) {
        g_calculator.display[0] = '0'; g_calculator.display[1] = '\0';
        g_calculator.operand_a = 0; g_calculator.operator = 0;
        g_calculator.has_operand = 0; g_calculator.next_clears = 0;
        calculator_draw(); return;
    }

    // Grid buttons
    uint32_t grid_y = disp_y + 50;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int32_t bx = (int32_t)(x + 6 + col * (BTN_W + BTN_PAD));
            int32_t by = (int32_t)(grid_y + row * (BTN_H + BTN_PAD));
            if (cx >= bx && cx < bx + BTN_W && cy >= by && cy < by + BTN_H) {
                const char* lbl = calc_labels[row][col];
                char ch = lbl[0];

                if (ch >= '0' && ch <= '9') {
                    if (g_calculator.next_clears) {
                        g_calculator.display[0] = ch; g_calculator.display[1] = '\0';
                        g_calculator.next_clears = 0;
                    } else {
                        int dlen = 0; while (g_calculator.display[dlen]) dlen++;
                        if (dlen < 15) { g_calculator.display[dlen] = ch; g_calculator.display[dlen+1] = '\0'; }
                    }
                } else if (ch == '.') {
                    int has_dot = 0;
                    for (int k = 0; g_calculator.display[k]; k++) if (g_calculator.display[k] == '.') { has_dot=1; break; }
                    if (!has_dot) {
                        int dlen = 0; while (g_calculator.display[dlen]) dlen++;
                        if (dlen < 15) { g_calculator.display[dlen] = '.'; g_calculator.display[dlen+1] = '\0'; }
                    }
                } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
                    g_calculator.operand_a  = simple_atof(g_calculator.display);
                    g_calculator.operator   = ch;
                    g_calculator.has_operand = 1;
                    g_calculator.next_clears = 1;
                } else if (ch == '=') {
                    if (g_calculator.has_operand) {
                        g_calculator.operand_b = simple_atof(g_calculator.display);
                        double res = 0;
                        if (g_calculator.operator == '+') res = g_calculator.operand_a + g_calculator.operand_b;
                        if (g_calculator.operator == '-') res = g_calculator.operand_a - g_calculator.operand_b;
                        if (g_calculator.operator == '*') res = g_calculator.operand_a * g_calculator.operand_b;
                        if (g_calculator.operator == '/' && g_calculator.operand_b != 0)
                            res = g_calculator.operand_a / g_calculator.operand_b;
                        // Display integer result
                        int_to_str(g_calculator.display, (int)res);
                        g_calculator.has_operand = 0;
                        g_calculator.next_clears = 1;
                    }
                }
                calculator_draw();
                return;
            }
        }
    }
}

/* ============================================================
 * Text Editor
 * ============================================================ */

#define ED_BG 0xFFFFF0

void texteditor_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    g_texteditor.x       = x;
    g_texteditor.y       = y;
    g_texteditor.width   = w;
    g_texteditor.height  = h;
    g_texteditor.buf_len = 0;
    g_texteditor.buf[0]  = '\0';
    g_texteditor.visible = 0;
    // Set default filename
    const char* fn = "untitled.txt";
    int i = 0; while (fn[i]) { g_texteditor.filename[i] = fn[i]; i++; } g_texteditor.filename[i] = '\0';
}

void texteditor_draw(void) {
    if (!g_texteditor.visible) return;
    uint32_t x = g_texteditor.x, y = g_texteditor.y;
    uint32_t w = g_texteditor.width, h = g_texteditor.height;
    draw_shadow_box(x, y, w, h, ED_BG);
    draw_titlebar(x, y, w, "Text Editor - untitled.txt", 0x6B3D8A);

    // Toolbar
    fill_rect(x + 2, y + 26, w - 4, 20, 0xDDDDDD);
    // [Save] button
    fill_rect(x + 6, y + 29, 48, 14, 0x5577AA);
    draw_rect(x + 6, y + 29, 48, 14, 0x334477);
    draw_string(x + 12, y + 33, "Save", 0xFFFFFF, 0x5577AA);

    // Text area
    fill_rect(x + 2, y + 48, w - 4, h - 52, ED_BG);
    draw_rect(x + 2, y + 48, w - 4, h - 52, 0xBBBBBB);

    uint32_t cx = x + 6, cy = y + 52;
    for (int i = 0; i < g_texteditor.buf_len; i++) {
        char c = g_texteditor.buf[i];
        if (c == '\n') { cx = x + 6; cy += 10; }
        else { draw_char(cx, cy, c, 0x000000, ED_BG); cx += 8; }
        if (cx + 8 > x + w - 6) { cx = x + 6; cy += 10; }
    }
    // Cursor
    fill_rect(cx, cy, 2, 9, 0x0000AA);
}

void texteditor_type(char c) {
    if (!g_texteditor.visible) return;
    if (c == '\b') {
        if (g_texteditor.buf_len > 0) g_texteditor.buf_len--;
    } else if (g_texteditor.buf_len < 4095) {
        g_texteditor.buf[g_texteditor.buf_len++] = c;
        g_texteditor.buf[g_texteditor.buf_len] = '\0';
    }
    texteditor_draw();
}

/* ============================================================
 * Browser
 * ============================================================ */

void browser_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    g_browser.x       = x;
    g_browser.y       = y;
    g_browser.width   = w;
    g_browser.height  = h;
    g_browser.visible = 0;
    const char* def_url = "http://www.lolos.org";
    int i = 0; while (def_url[i]) { g_browser.url[i] = def_url[i]; i++; } g_browser.url[i] = '\0';
}

void browser_draw(void) {
    if (!g_browser.visible) return;
    uint32_t x = g_browser.x, y = g_browser.y;
    uint32_t w = g_browser.width, h = g_browser.height;
    draw_shadow_box(x, y, w, h, 0xFFFFFF);
    draw_titlebar(x, y, w, "LoL Browser", 0x1E88E5);

    // Address bar
    fill_rect(x + 4, y + 26, w - 8, 22, 0xEEEEEE);
    draw_rect(x + 4, y + 26, w - 8, 22, 0xCCCCCC);
    draw_string(x + 10, y + 33, g_browser.url, 0x333333, 0xEEEEEE);

    // Content area
    fill_rect(x + 4, y + 52, w - 8, h - 56, 0xFFFFFF);
    draw_rect(x + 4, y + 52, w - 8, h - 56, 0xAAAAAA);

    draw_string(x + 20, y + 80, "Welcome to the Web!", 0x111111, 0xFFFFFF);
    draw_string(x + 20, y + 100, "LolOS Browser v0.1", 0x666666, 0xFFFFFF);
    
    // Some mock "web" content
    fill_rect(x + 20, y + 130, 100, 60, 0xBBDEFB);
    draw_rect(x + 20, y + 130, 100, 60, 0x1976D2);
    draw_string(x + 30, y + 150, "IMAGE", 0x1976D2, 0xBBDEFB);
    
    draw_string(x + 140, y + 130, "Latest News:", 0x000000, 0xFFFFFF);
    draw_string(x + 140, y + 150, "- System is blazing fast!", 0x2E7D32, 0xFFFFFF);
    draw_string(x + 140, y + 170, "- Browser is now available.", 0x1565C0, 0xFFFFFF);
}

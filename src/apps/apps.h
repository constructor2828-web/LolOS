#ifndef APPS_H
#define APPS_H

#include <stdint.h>

// ---- Terminal ----
typedef struct {
    uint32_t x, y, width, height;
    char     buf[2048];
    int      buf_len;
    uint8_t  visible;
} terminal_t;

// ---- File Manager ----
typedef struct {
    uint32_t x, y, width, height;
    int      selected;
    uint8_t  visible;
} filemanager_t;

// ---- Calculator ----
typedef struct {
    uint32_t x, y, width, height;
    char     display[20];
    double   operand_a;
    double   operand_b;
    char     operator;
    uint8_t  has_operand;
    uint8_t  next_clears;
    uint8_t  visible;
} calculator_t;

// ---- Text Editor ----
typedef struct {
    uint32_t x, y, width, height;
    char     buf[4096];
    int      buf_len;
    char     filename[32];
    uint8_t  visible;
} texteditor_t;

extern terminal_t    g_terminal;
extern filemanager_t g_filemanager;
extern calculator_t  g_calculator;
extern texteditor_t  g_texteditor;

void terminal_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void terminal_draw(void);
void terminal_type(char c);

void filemanager_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void filemanager_draw(void);

void calculator_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void calculator_draw(void);
void calculator_click(int32_t x, int32_t y);  // handle button click

void texteditor_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void texteditor_draw(void);
void texteditor_type(char c);

// ---- Browser ----
typedef struct {
    uint32_t x, y, width, height;
    char     url[256];
    uint16_t url_len;
    uint8_t  address_focused;
    uint8_t  visible;
} browser_t;

extern browser_t g_browser;

void browser_init(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void browser_draw(void);
void browser_click(int32_t x, int32_t y);
void browser_type(char c);

#endif

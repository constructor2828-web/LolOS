#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiboot.h"
#include "graphics.h"
#include "font.h"
#include "gui.h"
#include "apps.h"
#include "vfs.h"
#include "io.h"
#include "desktop.h"
#include "login.h"
#include "memory.h"
#include "rtl8139.h"
#include <stdint.h>

#define PORT 0x3f8   /* COM1 */

static void init_serial(void) {
   outb(PORT + 1, 0x00);
   outb(PORT + 3, 0x80);
   outb(PORT + 0, 0x03);
   outb(PORT + 1, 0x00);
   outb(PORT + 3, 0x03);
   outb(PORT + 2, 0xC7);
   outb(PORT + 4, 0x0B);
   outb(PORT + 4, 0x1E);
   outb(PORT + 0, 0xAE);
   if(inb(PORT + 0) != 0xAE) return;
   outb(PORT + 4, 0x0F);
}

static void write_serial(char a) {
   while ((inb(PORT + 5) & 0x20) == 0);
   outb(PORT, a);
}

static void print_serial(const char* str) {
    while (*str) { write_serial(*str++); }
}

static void print_hex(uint32_t num) {
    const char hex_chars[] = "0123456789ABCDEF";
    write_serial('0'); write_serial('x');
    for (int i = 7; i >= 0; i--)
        write_serial(hex_chars[(num >> (i * 4)) & 0xF]);
    write_serial('\n');
}

/* Keyboard handler routes chars to the active terminal */
void keyboard_set_handler(void (*h)(char));

void kernel_main(uint32_t magic, uint32_t addr) {
    init_serial();
    print_serial("\n=== LoLOS Kernel Starting ===\n");

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        print_serial("ERROR: Bad multiboot magic!\n");
        return;
    }

    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    print_serial("MBI Flags: ");
    print_hex(mbi->flags);

    /* Core init */
    init_gdt();
    pic_remap(0x20, 0x28);
    init_idt();

    /* Init Memory */
    init_memory(0);

    /* Init VFS */
    vfs_init();
    print_serial("VFS initialized.\n");

    // PCI & Network
    rtl8139_init();

    /* Graphics */
    if (mbi->flags & (1 << 12)) {
        print_serial("Framebuffer found - initializing graphics...\n");
        init_graphics(
            (uint32_t*)(uint32_t)mbi->framebuffer_addr,
            mbi->framebuffer_width,
            mbi->framebuffer_height,
            mbi->framebuffer_pitch
        );

        /* Draw desktop */
        draw_desktop();

        /* Draw desktop icons */
        draw_string(16, 50, "[>]", 0xFFFFFF, 0);
        draw_string(16, 60, "Terminal", 0xFFFFFF, 0);
        draw_string(16, 90, "[F]", 0xFFFFFF, 0);
        draw_string(16, 100, "Files", 0xFFFFFF, 0);

        /* Launch terminal window */
        terminal_init(200, 80, 500, 300);
        terminal_draw();

        /* Launch file manager window */
        filemanager_init(730, 80, 280, 400);
        filemanager_draw();

        print_serial("GUI rendered.\n");

        init_keyboard();
    extern void desktop_keyboard(char c);
    keyboard_set_handler(desktop_keyboard);

    /* Init mouse */
    init_mouse();
    print_serial("Mouse initialized.\n");
    
    // Initial draw
    if (os_state == OS_STATE_LOGIN) {
        login_draw();
    } else {
        desktop_draw();
    }
    swap_buffers();
} else {
    print_serial("WARNING: No framebuffer!\n");
    uint16_t* vga = (uint16_t*)0xB8000;
    const char* msg = "LoLOS booted - no VESA";
    for (int i = 0; msg[i]; i++)
        vga[i] = (uint16_t)(0x0F00 | (unsigned char)msg[i]);
    init_keyboard();
}

asm volatile("sti");
print_serial("Interrupts enabled. Kernel idle loop.\n");

while (1) {
    if (os_state == OS_STATE_LOGIN) {
        // Redraw cursor frequently on login screen since it doesn't have a desktop loop
        // (Actually mouse.c handles drawing, so hlt is fine)
    }
    asm volatile("hlt");
}
}

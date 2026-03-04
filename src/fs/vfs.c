#include "vfs.h"

static vfs_file_t vfs_table[MAX_FILES];
static int vfs_count_n = 0;

static int k_strlen(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void k_strcpy(char* dst, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

static int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

void vfs_init(void) {
    for (int i = 0; i < MAX_FILES; i++) vfs_table[i].used = 0;
    vfs_count_n = 0;

    // Pre-populate with sample files
    vfs_create("readme.txt", "Welcome to LoLOS!\nThis is a sample README file.");
    vfs_create("notes.txt", "LoLOS is an OS built in C and ASM.\nPhases: Boot, Core, GUI, Apps.");
    vfs_create("hello.txt", "Hello, World! This file lives in the LoLOS VFS.");
}

int vfs_create(const char* name, const char* content) {
    if (vfs_count_n >= MAX_FILES) return -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!vfs_table[i].used) {
            k_strcpy(vfs_table[i].name, name, MAX_FILENAME);
            int len = k_strlen(content);
            if (len >= MAX_FILESIZE) len = MAX_FILESIZE - 1;
            k_strcpy(vfs_table[i].data, content, MAX_FILESIZE);
            vfs_table[i].size = (uint32_t)len;
            vfs_table[i].used = 1;
            vfs_count_n++;
            return 0;
        }
    }
    return -1;
}

vfs_file_t* vfs_open(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs_table[i].used && k_strcmp(vfs_table[i].name, name) == 0)
            return &vfs_table[i];
    }
    return (vfs_file_t*)0;
}

int vfs_count(void) {
    return vfs_count_n;
}

vfs_file_t* vfs_get(int idx) {
    int n = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs_table[i].used) {
            if (n == idx) return &vfs_table[i];
            n++;
        }
    }
    return (vfs_file_t*)0;
}

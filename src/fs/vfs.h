#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_FILES     32
#define MAX_FILENAME  32
#define MAX_FILESIZE  512

typedef struct {
    char name[MAX_FILENAME];
    char data[MAX_FILESIZE];
    uint32_t size;
    uint8_t used;
} vfs_file_t;

void vfs_init(void);
int  vfs_create(const char* name, const char* content);
vfs_file_t* vfs_open(const char* name);
int  vfs_count(void);
vfs_file_t* vfs_get(int idx);

#endif

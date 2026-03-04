#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void init_memory(uint32_t start_addr);
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, uint32_t align);
void* memcpy(void* dest, const void* src, size_t n);

#endif

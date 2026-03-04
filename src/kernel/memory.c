#include "memory.h"

extern uint32_t end; // From linker script
static uint32_t placement_address;

void init_memory(uint32_t start_addr) {
    if (start_addr == 0) {
        placement_address = (uint32_t)&end;
    } else {
        placement_address = start_addr;
    }
}

void* kmalloc_aligned(size_t size, uint32_t align) {
    if (align > 0 && (placement_address % align) != 0) {
        placement_address += align - (placement_address % align);
    }
    uint32_t tmp = placement_address;
    placement_address += size;
    return (void*)tmp;
}

void* kmalloc(size_t size) {
    return kmalloc_aligned(size, 0);
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dest;
}

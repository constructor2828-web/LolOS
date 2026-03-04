#include "icons.h"

#define P 0xFF00FF // Transparent
#define W 0xFFFFFF // White
#define B 0x000000 // Black
#define G 0x00FF00 // Green
#define Y 0xFFFF00 // Yellow
#define D 0x444444 // Dark Gray
#define L 0x2222FF // Blue
#define R 0xFF0000 // Red

const uint32_t icon_terminal[256] = {
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,G,G,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,G,G,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,G,G,B,B,B,B,B,B,B,B,B,B,
    B,B,B,G,G,B,B,B,B,B,B,B,B,B,B,B,
    B,B,G,G,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,G,G,G,G,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
};

const uint32_t icon_files[256] = {
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,Y,Y,Y,P,P,P,P,P,P,P,P,P,P,P,
    P,Y,Y,Y,Y,Y,Y,Y,Y,P,P,P,P,P,P,P,
    Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,P,P,
    Y,W,W,W,W,W,W,W,W,W,W,W,W,Y,P,P,
    Y,W,W,W,W,W,W,W,W,W,W,W,W,Y,P,P,
    Y,W,W,W,W,W,W,W,W,W,W,W,W,Y,P,P,
    Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,P,P,
    Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,P,P,
    Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,P,P,
    Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
};

const uint32_t icon_calc[256] = {
    P,D,D,D,D,D,D,D,D,D,D,D,D,D,P,P,
    D,W,W,W,W,W,W,W,W,W,W,W,W,D,P,P,
    D,W,B,B,B,B,B,B,B,B,B,B,W,D,P,P,
    D,W,W,W,W,W,W,W,W,W,W,W,W,D,P,P,
    D,D,D,D,D,D,D,D,D,D,D,D,D,D,P,P,
    D,L,L,D,L,L,D,L,L,D,R,R,D,P,P,P,
    D,L,L,D,L,L,D,L,L,D,R,R,D,P,P,P,
    D,D,D,D,D,D,D,D,D,D,D,D,D,P,P,P,
    D,L,L,D,L,L,D,L,L,D,L,L,D,P,P,P,
    D,L,L,D,L,L,D,L,L,D,L,L,D,P,P,P,
    D,D,D,D,D,D,D,D,D,D,D,D,D,P,P,P,
    D,L,L,D,L,L,D,L,L,D,L,L,D,P,P,P,
    D,L,L,D,L,L,D,L,L,D,L,L,D,P,P,P,
    D,D,D,D,D,D,D,D,D,D,D,D,D,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
};

const uint32_t icon_editor[256] = {
    P,P,W,W,W,W,W,W,W,W,W,P,P,P,P,P,
    P,W,W,W,W,W,W,W,W,W,B,W,P,P,P,P,
    P,W,B,B,B,B,B,B,W,B,B,W,P,P,P,P,
    P,W,W,W,W,W,W,W,B,B,W,W,P,P,P,P,
    P,W,B,B,B,B,B,W,B,B,W,W,P,P,P,P,
    P,W,W,W,W,W,W,B,B,W,W,W,P,P,P,P,
    P,W,B,B,B,B,W,B,B,W,W,W,P,P,P,P,
    P,W,W,W,W,W,B,B,W,W,W,W,P,P,P,P,
    P,W,B,B,B,W,B,B,W,W,W,W,P,P,P,P,
    P,W,W,W,W,B,B,W,W,W,W,W,P,P,P,P,
    P,W,B,B,W,B,B,W,W,W,W,W,P,P,P,P,
    P,W,W,W,B,B,W,W,W,W,W,W,P,P,P,P,
    P,W,B,W,B,B,W,W,W,W,W,W,P,P,P,P,
    P,W,W,B,B,W,W,W,W,W,W,W,P,P,P,P,
    P,W,W,R,W,W,W,W,W,W,W,W,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
};

const uint32_t icon_browser[256] = {
    P,P,P,P,P,L,L,L,L,L,P,P,P,P,P,P,
    P,P,P,L,L,L,L,L,L,L,L,L,P,P,P,P,
    P,P,L,L,L,W,W,W,W,L,L,L,L,P,P,P,
    P,L,L,L,W,W,L,L,W,W,L,L,L,L,P,P,
    P,L,L,W,W,L,L,L,L,W,W,L,L,L,P,P,
    L,L,W,W,L,L,L,L,L,L,W,W,L,L,L,P,
    L,L,W,W,L,L,L,L,L,L,L,W,W,L,L,P,
    L,L,W,W,L,L,L,L,L,L,L,W,W,L,L,P,
    L,L,W,W,L,L,L,L,L,L,W,W,L,L,L,P,
    L,L,L,W,W,L,L,L,L,W,W,L,L,L,L,P,
    P,L,L,L,W,W,L,L,W,W,L,L,L,L,P,P,
    P,P,L,L,L,W,W,W,W,L,L,L,L,P,P,P,
    P,P,P,L,L,L,L,L,L,L,L,L,P,P,P,P,
    P,P,P,P,P,L,L,L,L,L,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
    P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,P,
};

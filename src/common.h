#ifndef COMMON_H
#define COMMON_H

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define TEX_SIZE 64
#define TEXELS_PER_TEXTURE (TEX_SIZE * TEX_SIZE)
#define MAX_TEXTURES 16
#define MAX_AMMO_TYPES 8
#define MAX_SPRITES 40
#define MAX_DOORS 16
#define MAX_TRIGGERS 24
#define MAX_ACTUATORS 48

#define TILE_EMPTY 0
#define TILE_DOOR 7

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct PaletteColor {
    u8 r;
    u8 g;
    u8 b;
} PaletteColor;

#endif

#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"

typedef struct PlatformInput {
    int quit;
    int forward;
    int backward;
    int turn_left;
    int turn_right;
    int strafe_left;
    int strafe_right;
    int use;
    int fire;
    int next_weapon;
    int mouse_delta_x;
} PlatformInput;

int platform_init(const char *title, const PaletteColor *palette);
void platform_shutdown(void);
void platform_poll_input(PlatformInput *input);
void platform_present(const u8 *framebuffer, const PaletteColor *palette);
u32 platform_ticks_ms(void);
void platform_sleep_ms(u32 ms);

#endif

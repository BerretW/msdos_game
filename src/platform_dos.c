#include "platform.h"
#include "config.h"

#include <conio.h>
#include <dos.h>
#include <memory.h>
#include <string.h>
#include <time.h>

static volatile u8 g_keys[128];
static void interrupt (*g_old_keyboard_isr)(void);
static u8 far *g_vga;

static int key_is_down(ConfigKey key)
{
    int scancode;

    scancode = -1;
    switch (key) {
    case CONFIG_KEY_W: scancode = 17; break;
    case CONFIG_KEY_A: scancode = 30; break;
    case CONFIG_KEY_S: scancode = 31; break;
    case CONFIG_KEY_D: scancode = 32; break;
    case CONFIG_KEY_M: scancode = 50; break;
    case CONFIG_KEY_Q: scancode = 16; break;
    case CONFIG_KEY_E: scancode = 18; break;
    case CONFIG_KEY_F: scancode = 33; break;
    case CONFIG_KEY_SPACE: scancode = 57; break;
    case CONFIG_KEY_TAB: scancode = 15; break;
    case CONFIG_KEY_ESCAPE: scancode = 1; break;
    case CONFIG_KEY_UP: scancode = 72; break;
    case CONFIG_KEY_DOWN: scancode = 80; break;
    case CONFIG_KEY_LEFT: scancode = 75; break;
    case CONFIG_KEY_RIGHT: scancode = 77; break;
    default: break;
    }

    return scancode >= 0 && g_keys[scancode] ? 1 : 0;
}

static void interrupt keyboard_isr(void)
{
    u8 scancode;

    scancode = inp(0x60);
    if (scancode & 0x80) {
        g_keys[scancode & 0x7F] = 0;
    } else {
        g_keys[scancode] = 1;
    }

    outp(0x20, 0x20);
}

static void set_video_mode(u8 mode)
{
    union REGS regs;

    regs.h.ah = 0x00;
    regs.h.al = mode;
    int86(0x10, &regs, &regs);
}

int platform_init(const char *title, const PaletteColor *palette)
{
    int i;

    (void)title;
    memset((void *)g_keys, 0, sizeof(g_keys));
    set_video_mode(0x13);
    g_vga = (u8 far *)MK_FP(0xA000, 0);

    outp(0x3C8, 0);
    for (i = 0; i < 256; ++i) {
        outp(0x3C9, palette[i].r >> 2);
        outp(0x3C9, palette[i].g >> 2);
        outp(0x3C9, palette[i].b >> 2);
    }

    g_old_keyboard_isr = getvect(0x09);
    setvect(0x09, keyboard_isr);
    return 1;
}

void platform_shutdown(void)
{
    if (g_old_keyboard_isr != 0) {
        setvect(0x09, g_old_keyboard_isr);
    }
    set_video_mode(0x03);
}

void platform_poll_input(PlatformInput *input)
{
    const GameConfig *config;
    int turn_left_down;
    int turn_right_down;
    int strafe_left_down;
    int strafe_right_down;

    memset(input, 0, sizeof(*input));
    config = config_get();
    turn_left_down = key_is_down(config->turn_left);
    turn_right_down = key_is_down(config->turn_right);
    strafe_left_down = key_is_down(config->strafe_left);
    strafe_right_down = key_is_down(config->strafe_right);
    input->quit = key_is_down(config->quit);
    input->forward = key_is_down(config->forward);
    input->backward = key_is_down(config->backward);
    if (config->mouse_look) {
        input->turn_left = 0;
        input->turn_right = 0;
        input->strafe_left = strafe_left_down || turn_left_down;
        input->strafe_right = strafe_right_down || turn_right_down;
    } else {
        input->turn_left = turn_left_down;
        input->turn_right = turn_right_down;
        input->strafe_left = strafe_left_down;
        input->strafe_right = strafe_right_down;
    }
    input->use = key_is_down(config->use);
    input->fire = key_is_down(config->fire);
    input->next_weapon = key_is_down(config->next_weapon);
    input->toggle_minimap = key_is_down(config->toggle_minimap);
    input->mouse_delta_x = 0;
}

void platform_present(const u8 *framebuffer, const PaletteColor *palette)
{
    (void)palette;
    _fmemcpy(g_vga, framebuffer, SCREEN_PIXELS);
}

u32 platform_ticks_ms(void)
{
    return (u32)((clock() * 1000UL) / CLOCKS_PER_SEC);
}

void platform_sleep_ms(u32 ms)
{
    u32 start;

    start = platform_ticks_ms();
    while ((platform_ticks_ms() - start) < ms) {
    }
}

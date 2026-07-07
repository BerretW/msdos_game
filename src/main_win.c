#include "game.h"
#include "platform.h"
#include "config.h"

static u8 g_framebuffer[SCREEN_PIXELS];

int main(void)
{
    const GameConfig *config;
    GameState game;
    PlatformInput input;
    u32 previous_ticks;
    u32 fps_accum_ms;
    u32 fps_frames;

    config_init();
    config = config_get();
    game_init(&game);
    if (!platform_init("Mode13h Prototype", game_palette())) {
        return 1;
    }

    previous_ticks = platform_ticks_ms();
    fps_accum_ms = 0;
    fps_frames = 0;

    for (;;) {
        u32 now;
        u32 delta;

        platform_poll_input(&input);
        if (input.quit) {
            break;
        }

        now = platform_ticks_ms();
        delta = now - previous_ticks;
        if (delta > 50) {
            delta = 50;
        }
        previous_ticks = now;

        fps_accum_ms += delta;
        fps_frames += 1;
        if (fps_accum_ms >= 250) {
            game.fps_display = (int)((fps_frames * 1000UL) / fps_accum_ms);
            fps_accum_ms = 0;
            fps_frames = 0;
        }

        game_update(&game, &input, delta);
        game_render(&game, g_framebuffer);
        platform_present(g_framebuffer, game_palette());
        if (config->fps_cap > 0) {
            u32 target_ms;
            u32 elapsed_ms;

            target_ms = 1000UL / (u32)config->fps_cap;
            if (target_ms < 1) {
                target_ms = 1;
            }
            elapsed_ms = platform_ticks_ms() - now;
            if (elapsed_ms < target_ms) {
                platform_sleep_ms(target_ms - elapsed_ms);
            }
        }
    }

    platform_shutdown();
    return 0;
}

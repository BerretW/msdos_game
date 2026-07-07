#include "assets.h"

#include <stdio.h>
#include <string.h>

static u8 g_textures[MAX_TEXTURES][TEXELS_PER_TEXTURE];
static int g_loaded[MAX_TEXTURES];
static PaletteColor g_palette[256];

static FILE *open_data_file(const char *path, char *resolved_path)
{
    static const char *prefixes[] = {"", "../"};
    FILE *file;
    int i;

    for (i = 0; i < 2; ++i) {
        sprintf(resolved_path, "%s%s", prefixes[i], path);
        file = fopen(resolved_path, "rb");
        if (file != 0) {
            return file;
        }
    }

    resolved_path[0] = '\0';
    return 0;
}

static void set_palette_range(int start, int end, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
    int i;
    int span;

    span = end - start;
    if (span <= 0) {
        return;
    }

    for (i = start; i <= end; ++i) {
        int t_num;
        int t_den;

        t_num = i - start;
        t_den = span;
        g_palette[i].r = (u8)(r0 + ((r1 - r0) * t_num) / t_den);
        g_palette[i].g = (u8)(g0 + ((g1 - g0) * t_num) / t_den);
        g_palette[i].b = (u8)(b0 + ((b1 - b0) * t_num) / t_den);
    }
}

static void build_fallback_texture(void)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;
            int cell;

            index = y * TEX_SIZE + x;
            cell = ((x >> 3) ^ (y >> 3)) & 1;
            if (x == y || x == (TEX_SIZE - 1 - y)) {
                g_textures[0][index] = 250;
            } else {
                g_textures[0][index] = cell ? 245 : 240;
            }
        }
    }
    g_loaded[0] = 1;
}

static void build_brick_texture(int texture_id, u8 base, u8 mortar)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;
            int brick_row;
            int brick_col;

            index = y * TEX_SIZE + x;
            brick_row = (y >> 3) & 1;
            brick_col = ((x + (brick_row ? 4 : 0)) >> 4) & 1;

            if ((y & 7) == 0 || (x & 15) == 0) {
                g_textures[texture_id][index] = mortar;
            } else {
                g_textures[texture_id][index] = (u8)(base + brick_col + ((x + y) & 1));
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_runic_texture(int texture_id, u8 dark, u8 light, u8 rune)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;
            int ring;

            index = y * TEX_SIZE + x;
            ring = ((x - 32) * (x - 32) + (y - 32) * (y - 32));

            if ((x > 27 && x < 37 && y > 9 && y < 55) || (y > 27 && y < 37 && x > 9 && x < 55)) {
                g_textures[texture_id][index] = rune;
            } else if (ring < 760) {
                g_textures[texture_id][index] = light;
            } else {
                g_textures[texture_id][index] = dark;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_panel_texture(int texture_id, u8 bg, u8 stripe, u8 trim)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            if (x < 4 || y < 4 || x >= 60 || y >= 60) {
                g_textures[texture_id][index] = trim;
            } else if (((x + y) & 7) < 2) {
                g_textures[texture_id][index] = stripe;
            } else {
                g_textures[texture_id][index] = bg;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_barrel_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;
            int dx;
            int dy;
            int body;

            index = y * TEX_SIZE + x;
            dx = x - 32;
            dy = y - 34;
            body = dx * dx + dy * dy;
            g_textures[texture_id][index] = 0;
            if (body < 400 && y > 12 && y < 58) {
                if (y < 18 || y > 50 || (x > 18 && x < 46 && ((y >> 2) & 1) == 0)) {
                    g_textures[texture_id][index] = 196;
                } else {
                    g_textures[texture_id][index] = 136;
                }
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_enemy_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if ((x - 32) * (x - 32) + (y - 16) * (y - 16) < 80) {
                g_textures[texture_id][index] = 232;
            } else if (x > 20 && x < 44 && y > 18 && y < 48) {
                g_textures[texture_id][index] = 160;
            } else if (x > 16 && x < 24 && y > 24 && y < 50) {
                g_textures[texture_id][index] = 192;
            } else if (x > 40 && x < 48 && y > 24 && y < 50) {
                g_textures[texture_id][index] = 192;
            } else if (x > 24 && x < 40 && y > 48 && y < 60) {
                g_textures[texture_id][index] = 196;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_door_texture(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            if (x < 5 || x > 58 || y < 5 || y > 58) {
                g_textures[texture_id][index] = 220;
            } else if ((x > 28 && x < 36) || (y > 28 && y < 36)) {
                g_textures[texture_id][index] = 232;
            } else if (((x + y) & 15) < 3) {
                g_textures[texture_id][index] = 164;
            } else {
                g_textures[texture_id][index] = 170;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_potion_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if ((x - 32) * (x - 32) + (y - 20) * (y - 20) < 60) {
                g_textures[texture_id][index] = 232;
            } else if (x > 22 && x < 42 && y > 24 && y < 52) {
                g_textures[texture_id][index] = 72;
            } else if (x > 20 && x < 44 && y > 46 && y < 58) {
                g_textures[texture_id][index] = 220;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_knight_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if ((x - 32) * (x - 32) + (y - 14) * (y - 14) < 90) {
                g_textures[texture_id][index] = 224;
            } else if (x > 18 && x < 46 && y > 18 && y < 50) {
                g_textures[texture_id][index] = 104;
            } else if (x > 14 && x < 22 && y > 24 && y < 56) {
                g_textures[texture_id][index] = 192;
            } else if (x > 42 && x < 50 && y > 22 && y < 56) {
                g_textures[texture_id][index] = 192;
            } else if (x > 24 && x < 40 && y > 48 && y < 61) {
                g_textures[texture_id][index] = 196;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_ammo_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if (x > 18 && x < 46 && y > 18 && y < 42) {
                g_textures[texture_id][index] = 196;
            } else if (x > 22 && x < 42 && y > 12 && y < 20) {
                g_textures[texture_id][index] = 220;
            } else if (x > 24 && x < 40 && y > 42 && y < 54) {
                g_textures[texture_id][index] = 164;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_key_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if ((x - 22) * (x - 22) + (y - 22) * (y - 22) < 80) {
                g_textures[texture_id][index] = 224;
            } else if (x > 22 && x < 46 && y > 20 && y < 28) {
                g_textures[texture_id][index] = 224;
            } else if (x > 38 && x < 46 && y > 24 && y < 50) {
                g_textures[texture_id][index] = 224;
            } else if (x > 44 && x < 54 && y > 30 && y < 36) {
                g_textures[texture_id][index] = 220;
            } else if (x > 44 && x < 52 && y > 40 && y < 46) {
                g_textures[texture_id][index] = 220;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_trigger_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if (x > 16 && x < 48 && y > 16 && y < 48) {
                if (x < 22 || x > 42 || y < 22 || y > 42) {
                    g_textures[texture_id][index] = 220;
                } else {
                    g_textures[texture_id][index] = 232;
                }
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_exit_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if (x > 14 && x < 50 && y > 14 && y < 50) {
                g_textures[texture_id][index] = ((x + y) & 6) ? 72 : 96;
            }
            if (x > 26 && x < 38 && y > 10 && y < 54) {
                g_textures[texture_id][index] = 224;
            }
            if (y > 26 && y < 38 && x > 10 && x < 54) {
                g_textures[texture_id][index] = 224;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_weapon_pickup_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;

            index = y * TEX_SIZE + x;
            g_textures[texture_id][index] = 0;
            if (x > 10 && x < 52 && y > 28 && y < 38) {
                g_textures[texture_id][index] = 196;
            }
            if (x > 18 && x < 46 && y > 22 && y < 30) {
                g_textures[texture_id][index] = 224;
            }
            if (x > 24 && x < 40 && y > 38 && y < 50) {
                g_textures[texture_id][index] = 164;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

static void build_projectile_sprite(int texture_id)
{
    int x;
    int y;

    for (y = 0; y < TEX_SIZE; ++y) {
        for (x = 0; x < TEX_SIZE; ++x) {
            int index;
            int dx;
            int dy;

            index = y * TEX_SIZE + x;
            dx = x - 32;
            dy = y - 32;
            g_textures[texture_id][index] = 0;
            if (dx * dx + dy * dy < 90) {
                g_textures[texture_id][index] = 232;
            }
            if (dx * dx + dy * dy < 32) {
                g_textures[texture_id][index] = 224;
            }
        }
    }
    g_loaded[texture_id] = 1;
}

void assets_init(void)
{
    memset(g_textures, 0, sizeof(g_textures));
    memset(g_loaded, 0, sizeof(g_loaded));
    memset(g_palette, 0, sizeof(g_palette));

    set_palette_range(0, 31, 0, 0, 0, 40, 52, 88);
    set_palette_range(32, 63, 26, 18, 12, 120, 92, 48);
    set_palette_range(64, 95, 8, 24, 10, 110, 160, 78);
    set_palette_range(96, 127, 20, 16, 16, 140, 130, 128);
    set_palette_range(128, 159, 16, 8, 8, 148, 54, 28);
    set_palette_range(160, 191, 16, 18, 30, 148, 148, 168);
    set_palette_range(192, 223, 8, 8, 8, 220, 220, 220);
    set_palette_range(224, 239, 24, 20, 12, 240, 212, 132);
    set_palette_range(240, 255, 0, 0, 0, 255, 0, 255);

    build_fallback_texture();
    build_brick_texture(1, 128, 196);
    build_runic_texture(2, 96, 110, 232);
    build_panel_texture(3, 66, 72, 220);
    build_barrel_sprite(5);
    build_enemy_sprite(6);
    build_door_texture(7);
    build_potion_sprite(8);
    build_knight_sprite(9);
    build_ammo_sprite(10);
    build_key_sprite(11);
    build_trigger_sprite(12);
    build_exit_sprite(13);
    build_weapon_pickup_sprite(14);
    build_projectile_sprite(15);
}

void assets_load_manifest(const char *path)
{
    FILE *file;
    char resolved_path[260];
    char base_path[260];
    char line[260];

    file = open_data_file(path, resolved_path);
    if (file == 0) {
        return;
    }

    strcpy(base_path, resolved_path);
    {
        char *slash;

        slash = strrchr(base_path, '/');
        if (slash == 0) {
            slash = strrchr(base_path, '\\');
        }
        if (slash != 0) {
            slash[1] = '\0';
        } else {
            base_path[0] = '\0';
        }
    }

    while (fgets(line, sizeof(line), file) != 0) {
        int texture_id;
        char texture_name[128];
        char texture_path[260];

        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (sscanf(line, "%d %127s", &texture_id, texture_name) == 2) {
            sprintf(texture_path, "%s%s", base_path, texture_name);
            assets_try_load_raw(texture_id, texture_path);
        }
    }

    fclose(file);
}

void assets_try_load_raw(int texture_id, const char *path)
{
    FILE *file;
    size_t read_count;
    char resolved_path[260];

    if (texture_id <= 0 || texture_id >= MAX_TEXTURES || path == 0) {
        return;
    }

    file = open_data_file(path, resolved_path);
    if (file == 0) {
        return;
    }

    read_count = fread(g_textures[texture_id], 1, TEXELS_PER_TEXTURE, file);
    fclose(file);

    if (read_count == TEXELS_PER_TEXTURE) {
        g_loaded[texture_id] = 1;
    }
}

const u8 *assets_get_texture(int texture_id)
{
    if (texture_id <= 0 || texture_id >= MAX_TEXTURES) {
        return g_textures[0];
    }

    if (!g_loaded[texture_id]) {
        return g_textures[0];
    }

    return g_textures[texture_id];
}

const PaletteColor *assets_get_palette(void)
{
    return g_palette;
}

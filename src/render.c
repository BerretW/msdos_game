#include "render.h"

#include "assets.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static double g_depth_buffer[SCREEN_WIDTH];

static int glyph_row_bits(char ch, int row)
{
    static const unsigned long digits[10] = {
        0x7B6F, 0x2492, 0x73E7, 0x73CF, 0x5F11,
        0x79CF, 0x79EF, 0x7249, 0x7BEF, 0x7BCF
    };
    unsigned long bits;

    if (row < 0 || row >= 5) {
        return 0;
    }

    if (ch >= '0' && ch <= '9') {
        bits = digits[ch - '0'];
        return (bits >> ((4 - row) * 3)) & 7;
    }

    switch (ch) {
    case 'A': bits = 0x7BF7; break;
    case 'E': bits = 0x79E79; break;
    case 'F': bits = 0x79E49; break;
    case 'H': bits = 0x5BF5B; break;
    case 'I': bits = 0x72427; break;
    case 'K': bits = 0x5D75D; break;
    case 'M': bits = 0x5FF5B; break;
    case 'N': bits = 0x5FB7B; break;
    case 'O': bits = 0x7B6F7; break;
    case 'P': bits = 0x7BE49; break;
    case 'R': bits = 0x7BEDB; break;
    case 'S': bits = 0x79C2F; break;
    case 'T': bits = 0x72492; break;
    case 'V': bits = 0x5B6D2; break;
    case 'W': bits = 0x5B7FF; break;
    case 'X': bits = 0x5AD5A; break;
    case 'Y': bits = 0x5AD22; break;
    case ':': bits = 0x02020; break;
    case '.': bits = 0x00002; break;
    case ',': bits = 0x00012; break;
    case '-': bits = 0x00070; break;
    default: bits = 0x00000; break;
    }

    return (bits >> ((4 - row) * 3)) & 7;
}

static void draw_rect(u8 *framebuffer, int x0, int y0, int x1, int y1, u8 color)
{
    int x;
    int y;

    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 > SCREEN_WIDTH) {
        x1 = SCREEN_WIDTH;
    }
    if (y1 > SCREEN_HEIGHT) {
        y1 = SCREEN_HEIGHT;
    }

    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

static void draw_char(u8 *framebuffer, int x, int y, char ch, u8 color)
{
    int row;

    for (row = 0; row < 5; ++row) {
        int bits;
        int col;

        bits = glyph_row_bits(ch, row);
        for (col = 0; col < 3; ++col) {
            if ((bits & (1 << (2 - col))) != 0) {
                int px;
                int py;

                px = x + col;
                py = y + row;
                if (px >= 0 && py >= 0 && px < SCREEN_WIDTH && py < SCREEN_HEIGHT) {
                    framebuffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
    }
}

static void draw_text(u8 *framebuffer, int x, int y, const char *text, u8 color)
{
    int cursor_x;

    cursor_x = x;
    while (*text != '\0') {
        if (*text != ' ') {
            draw_char(framebuffer, cursor_x, y, *text, color);
        }
        cursor_x += 4;
        ++text;
    }
}

static void draw_outline_rect(u8 *framebuffer, int x0, int y0, int x1, int y1, u8 edge, u8 fill)
{
    draw_rect(framebuffer, x0, y0, x1, y1, fill);
    draw_rect(framebuffer, x0, y0, x1, y0 + 1, edge);
    draw_rect(framebuffer, x0, y1 - 1, x1, y1, edge);
    draw_rect(framebuffer, x0, y0, x0 + 1, y1, edge);
    draw_rect(framebuffer, x1 - 1, y0, x1, y1, edge);
}

static void draw_minimap(const GameState *game, u8 *framebuffer)
{
    int map_w;
    int map_h;
    int max_map_size;
    int cell_size;
    int map_px_w;
    int map_px_h;
    int origin_x;
    int origin_y;
    int x;
    int y;

    map_w = game->map_width;
    map_h = game->map_height;
    if (map_w <= 0 || map_h <= 0) {
        return;
    }

    max_map_size = map_w > map_h ? map_w : map_h;
    cell_size = 2;
    if (max_map_size <= 24) {
        cell_size = 4;
    } else if (max_map_size <= 40) {
        cell_size = 3;
    }

    map_px_w = map_w * cell_size;
    map_px_h = map_h * cell_size;
    origin_x = SCREEN_WIDTH - map_px_w - 6;
    origin_y = 6;

    if (origin_x < 160) {
        origin_x = 160;
    }
    if (origin_y + map_px_h + 2 > SCREEN_HEIGHT - 20) {
        return;
    }

    draw_outline_rect(framebuffer, origin_x - 2, origin_y - 2, origin_x + map_px_w + 2, origin_y + map_px_h + 2, 220, 16);

    for (y = 0; y < map_h; ++y) {
        for (x = 0; x < map_w; ++x) {
            int tile;
            u8 color;

            tile = game_map_at(x, y);
            color = 24;
            if (tile == TILE_DOOR) {
                color = 180;
            } else if (tile != TILE_EMPTY) {
                color = 88;
            }

            draw_rect(
                framebuffer,
                origin_x + x * cell_size,
                origin_y + y * cell_size,
                origin_x + (x + 1) * cell_size,
                origin_y + (y + 1) * cell_size,
                color);
        }
    }

    for (x = 0; x < game->sprite_count; ++x) {
        const SpriteState *sprite;
        int sprite_px;
        int sprite_py;
        u8 color;

        sprite = &game->sprites[x];
        if (!sprite->active) {
            continue;
        }

        sprite_px = origin_x + (int)(sprite->x * (double)cell_size);
        sprite_py = origin_y + (int)(sprite->y * (double)cell_size);
        color = sprite->is_enemy ? 248 : 168;
        if (sprite->pickup_key_mask != 0 || sprite->pickup_weapon_id != 0) {
            color = 224;
        }
        if (sprite->is_projectile) {
            color = 240;
        }

        draw_rect(framebuffer, sprite_px, sprite_py, sprite_px + cell_size, sprite_py + cell_size, color);
    }

    {
        int player_px;
        int player_py;
        int dir_px;
        int dir_py;

        player_px = origin_x + (int)(game->player_x * (double)cell_size);
        player_py = origin_y + (int)(game->player_y * (double)cell_size);
        dir_px = player_px + (int)(game->dir_x * (double)(cell_size * 2));
        dir_py = player_py + (int)(game->dir_y * (double)(cell_size * 2));

        draw_rect(framebuffer, player_px - 1, player_py - 1, player_px + cell_size, player_py + cell_size, 252);
        draw_rect(framebuffer, dir_px, dir_py, dir_px + 1, dir_py + 1, 240);
    }
}

static void draw_debug_panel(const GameState *game, u8 *framebuffer)
{
    char line0[64];
    char line1[64];
    char line2[64];
    char owned[8];
    int owned_len;
    int weapon_id;
    int tile_x;
    int tile_y;
    int ammo_type_id;

    owned_len = 0;
    for (weapon_id = 1; weapon_id <= 4; ++weapon_id) {
        if (game_weapon_owned(game, weapon_id) && owned_len < (int)sizeof(owned) - 1) {
            owned[owned_len++] = (char)('0' + weapon_id);
        }
    }
    owned[owned_len] = '\0';
    if (owned_len == 0) {
        strcpy(owned, "-");
    }

    tile_x = (int)game->player_x;
    tile_y = (int)game->player_y;
    ammo_type_id = game_player_ammo_type(game);
    sprintf(line0, "POS X%02d.%d Y%02d.%d", tile_x, ((int)(game->player_x * 10.0)) % 10, tile_y, ((int)(game->player_y * 10.0)) % 10);
    sprintf(line1, "HP %03d A%d %02d FPS %03d", game->health, ammo_type_id, game_player_ammo(game), game->fps_display);
    sprintf(line2, "INV K:%d W:%d O:%s", game->keys_mask, game->current_weapon_id, owned);

    draw_outline_rect(framebuffer, 4, 4, 158, 31, 220, 32);
    draw_text(framebuffer, 8, 8, line0, 224);
    draw_text(framebuffer, 8, 15, line1, 196);
    draw_text(framebuffer, 8, 22, line2, 168);
}

static int trace_door_hit(
    const GameState *game,
    double player_x,
    double player_y,
    double ray_dir_x,
    double ray_dir_y,
    int map_x,
    int map_y,
    double *perp_wall_dist,
    double *wall_x)
{
    double openness;
    double door_plane_x;
    double hit_t;
    double hit_y;
    double frac_y;

    (void)game;
    openness = game_door_openness_at(game, map_x, map_y);
    if (openness >= 0.999 || ray_dir_x == 0.0) {
        return 0;
    }

    door_plane_x = (double)map_x + openness;
    hit_t = (door_plane_x - player_x) / ray_dir_x;
    if (hit_t <= 0.0) {
        return 0;
    }

    hit_y = player_y + hit_t * ray_dir_y;
    frac_y = hit_y - floor(hit_y);
    if (frac_y < 0.0 || frac_y >= 1.0) {
        return 0;
    }

    *perp_wall_dist = hit_t;
    *wall_x = frac_y;
    return 1;
}

static void clear_framebuffer(u8 *framebuffer)
{
    int x;
    int y;

    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        u8 color;

        color = y < (SCREEN_HEIGHT / 2) ? 18 : 34;
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

static void draw_crosshair(u8 *framebuffer)
{
    int cx;
    int cy;
    int i;

    cx = SCREEN_WIDTH / 2;
    cy = SCREEN_HEIGHT / 2;

    for (i = -4; i <= 4; ++i) {
        framebuffer[cy * SCREEN_WIDTH + (cx + i)] = 224;
        framebuffer[(cy + i) * SCREEN_WIDTH + cx] = 224;
    }
}

static void draw_status_bar(const GameState *game, u8 *framebuffer)
{
    int x;
    int y;
    int health_width;
    int ammo_width;
    int has_key;
    int current_weapon;

    for (y = SCREEN_HEIGHT - 18; y < SCREEN_HEIGHT; ++y) {
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            framebuffer[y * SCREEN_WIDTH + x] = 96;
        }
    }

    health_width = game_player_health(game) * 3;
    if (health_width > 300) {
        health_width = 300;
    }

    for (y = SCREEN_HEIGHT - 14; y < SCREEN_HEIGHT - 6; ++y) {
        for (x = 10; x < 10 + health_width; ++x) {
            framebuffer[y * SCREEN_WIDTH + x] = 224;
        }
    }

    ammo_width = game_player_ammo(game) * 2;
    if (ammo_width > 120) {
        ammo_width = 120;
    }
    for (y = SCREEN_HEIGHT - 14; y < SCREEN_HEIGHT - 6; ++y) {
        for (x = 170; x < 170 + ammo_width; ++x) {
            framebuffer[y * SCREEN_WIDTH + x] = 196;
        }
    }

    has_key = game_player_keys(game) & 1;
    for (y = SCREEN_HEIGHT - 14; y < SCREEN_HEIGHT - 6; ++y) {
        for (x = 300; x < 310; ++x) {
            framebuffer[y * SCREEN_WIDTH + x] = has_key ? 224 : 32;
        }
    }

    current_weapon = game_current_weapon(game);
    for (x = 0; x < 4; ++x) {
        int weapon_id;
        int box_x0;
        int box_x1;
        u8 fill_color;
        u8 edge_color;

        weapon_id = x + 1;
        box_x0 = 238 + x * 14;
        box_x1 = box_x0 + 10;
        fill_color = game_weapon_owned(game, weapon_id) ? 168 : 48;
        edge_color = current_weapon == weapon_id ? 232 : 96;

        for (y = SCREEN_HEIGHT - 15; y < SCREEN_HEIGHT - 5; ++y) {
            int px;

            for (px = box_x0; px < box_x1; ++px) {
                if (y == SCREEN_HEIGHT - 15 || y == SCREEN_HEIGHT - 6 || px == box_x0 || px == box_x1 - 1) {
                    framebuffer[y * SCREEN_WIDTH + px] = edge_color;
                } else {
                    framebuffer[y * SCREEN_WIDTH + px] = fill_color;
                }
            }
        }
    }

    draw_debug_panel(game, framebuffer);
    draw_minimap(game, framebuffer);

    if (game_weapon_flash(game) > 0) {
        for (y = SCREEN_HEIGHT - 40; y < SCREEN_HEIGHT - 18; ++y) {
            for (x = 140; x < 180; ++x) {
                framebuffer[y * SCREEN_WIDTH + x] = 240;
            }
        }
    }

    if (game_damage_flash(game) > 0) {
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            framebuffer[x] = 248;
            framebuffer[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x] = 248;
        }
        for (y = 0; y < SCREEN_HEIGHT; ++y) {
            framebuffer[y * SCREEN_WIDTH] = 248;
            framebuffer[y * SCREEN_WIDTH + (SCREEN_WIDTH - 1)] = 248;
        }
    }
}

static void render_sprites(const GameState *game, u8 *framebuffer)
{
    int order[MAX_SPRITES];
    double distance[MAX_SPRITES];
    int count;
    int i;

    count = game->sprite_count;
    if (count > MAX_SPRITES) {
        count = MAX_SPRITES;
    }

    for (i = 0; i < count; ++i) {
        double dx;
        double dy;

        order[i] = i;
        dx = game->player_x - game->sprites[i].x;
        dy = game->player_y - game->sprites[i].y;
        distance[i] = dx * dx + dy * dy;
    }

    for (i = 0; i < count - 1; ++i) {
        int j;

        for (j = i + 1; j < count; ++j) {
            if (distance[j] > distance[i]) {
                double tmp_dist;
                int tmp_order;

                tmp_dist = distance[i];
                distance[i] = distance[j];
                distance[j] = tmp_dist;
                tmp_order = order[i];
                order[i] = order[j];
                order[j] = tmp_order;
            }
        }
    }

    for (i = 0; i < count; ++i) {
        const SpriteState *sprite;
        double sprite_x;
        double sprite_y;
        double inv_det;
        double transform_x;
        double transform_y;
        int sprite_screen_x;
        int sprite_height;
        int sprite_width;
        int draw_start_y;
        int draw_end_y;
        int draw_start_x;
        int draw_end_x;
        const u8 *texture;
        int stripe;

        sprite = &game->sprites[order[i]];
        if (!sprite->active) {
            continue;
        }

        sprite_x = sprite->x - game->player_x;
        sprite_y = sprite->y - game->player_y;
        inv_det = 1.0 / (game->plane_x * game->dir_y - game->dir_x * game->plane_y);
        transform_x = inv_det * (game->dir_y * sprite_x - game->dir_x * sprite_y);
        transform_y = inv_det * (-game->plane_y * sprite_x + game->plane_x * sprite_y);
        if (transform_y <= 0.1) {
            continue;
        }

        sprite_screen_x = (int)((SCREEN_WIDTH / 2) * (1.0 + transform_x / transform_y));
        sprite_height = (int)fabs((double)(SCREEN_HEIGHT / transform_y));
        sprite_width = sprite_height;
        draw_start_y = -sprite_height / 2 + SCREEN_HEIGHT / 2;
        draw_end_y = sprite_height / 2 + SCREEN_HEIGHT / 2;
        draw_start_x = -sprite_width / 2 + sprite_screen_x;
        draw_end_x = sprite_width / 2 + sprite_screen_x;

        if (draw_start_y < 0) {
            draw_start_y = 0;
        }
        if (draw_end_y >= SCREEN_HEIGHT) {
            draw_end_y = SCREEN_HEIGHT - 1;
        }
        if (draw_start_x < 0) {
            draw_start_x = 0;
        }
        if (draw_end_x >= SCREEN_WIDTH) {
            draw_end_x = SCREEN_WIDTH - 1;
        }

        texture = assets_get_texture(sprite->texture_id);
        for (stripe = draw_start_x; stripe <= draw_end_x; ++stripe) {
            int tex_x;

            tex_x = (stripe - (-sprite_width / 2 + sprite_screen_x)) * TEX_SIZE / sprite_width;
            if (transform_y >= g_depth_buffer[stripe]) {
                continue;
            }

            {
                int y;

                for (y = draw_start_y; y <= draw_end_y; ++y) {
                    int d;
                    int tex_y;
                    u8 color;

                    d = y * 256 - SCREEN_HEIGHT * 128 + sprite_height * 128;
                    tex_y = ((d * TEX_SIZE) / sprite_height) / 256;
                    if (tex_y < 0) {
                        tex_y = 0;
                    }
                    if (tex_y >= TEX_SIZE) {
                        tex_y = TEX_SIZE - 1;
                    }

                    color = texture[tex_y * TEX_SIZE + tex_x];
                    if (color != 0) {
                        framebuffer[y * SCREEN_WIDTH + stripe] = color;
                    }
                }
            }
        }
    }
}

void render_world(const GameState *game, u8 *framebuffer)
{
    int x;

    clear_framebuffer(framebuffer);

    for (x = 0; x < SCREEN_WIDTH; ++x) {
        double camera_x;
        double ray_dir_x;
        double ray_dir_y;
        int map_x;
        int map_y;
        double side_dist_x;
        double side_dist_y;
        double delta_dist_x;
        double delta_dist_y;
        double perp_wall_dist;
        int step_x;
        int step_y;
        int hit;
        int side;
        int line_height;
        int draw_start;
        int draw_end;
        int tex_id;
        const u8 *texture;
        double wall_x;
        int tex_x;
        int used_custom_wall_x;
        int y;

        camera_x = (2.0 * x / (double)SCREEN_WIDTH) - 1.0;
        ray_dir_x = game->dir_x + game->plane_x * camera_x;
        ray_dir_y = game->dir_y + game->plane_y * camera_x;
        map_x = (int)game->player_x;
        map_y = (int)game->player_y;

        delta_dist_x = (ray_dir_x == 0.0) ? 1e30 : 1.0 / fabs(ray_dir_x);
        delta_dist_y = (ray_dir_y == 0.0) ? 1e30 : 1.0 / fabs(ray_dir_y);

        if (ray_dir_x < 0.0) {
            step_x = -1;
            side_dist_x = (game->player_x - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0 - game->player_x) * delta_dist_x;
        }

        if (ray_dir_y < 0.0) {
            step_y = -1;
            side_dist_y = (game->player_y - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0 - game->player_y) * delta_dist_y;
        }

        hit = 0;
        side = 0;
        used_custom_wall_x = 0;
        while (!hit) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }

            tex_id = game_map_at(map_x, map_y);
            if (tex_id == TILE_DOOR) {
                if (trace_door_hit(game, game->player_x, game->player_y, ray_dir_x, ray_dir_y, map_x, map_y, &perp_wall_dist, &wall_x)) {
                    hit = 1;
                    side = 0;
                    used_custom_wall_x = 1;
                }
            } else if (tex_id != TILE_EMPTY) {
                hit = 1;
            }
        }

        if (!used_custom_wall_x && side == 0) {
            perp_wall_dist = (map_x - game->player_x + (1.0 - step_x) * 0.5) / ray_dir_x;
        } else if (!used_custom_wall_x) {
            perp_wall_dist = (map_y - game->player_y + (1.0 - step_y) * 0.5) / ray_dir_y;
        }

        if (perp_wall_dist < 0.001) {
            perp_wall_dist = 0.001;
        }

        line_height = (int)(SCREEN_HEIGHT / perp_wall_dist);
        draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
        draw_end = line_height / 2 + SCREEN_HEIGHT / 2;

        if (draw_start < 0) {
            draw_start = 0;
        }
        if (draw_end >= SCREEN_HEIGHT) {
            draw_end = SCREEN_HEIGHT - 1;
        }

        if (!used_custom_wall_x && side == 0) {
            wall_x = game->player_y + perp_wall_dist * ray_dir_y;
        } else if (!used_custom_wall_x) {
            wall_x = game->player_x + perp_wall_dist * ray_dir_x;
        }
        if (!used_custom_wall_x) {
            wall_x -= (double)((int)wall_x);
        }

        tex_x = (int)(wall_x * (double)TEX_SIZE);
        if (tex_x < 0) {
            tex_x = 0;
        }
        if (tex_x >= TEX_SIZE) {
            tex_x = TEX_SIZE - 1;
        }
        if (side == 0 && ray_dir_x > 0.0) {
            tex_x = TEX_SIZE - tex_x - 1;
        }
        if (side == 1 && ray_dir_y < 0.0) {
            tex_x = TEX_SIZE - tex_x - 1;
        }

        texture = assets_get_texture(tex_id);
        for (y = draw_start; y <= draw_end; ++y) {
            int d;
            int tex_y;
            u8 color;

            d = y * 256 - SCREEN_HEIGHT * 128 + line_height * 128;
            tex_y = ((d * TEX_SIZE) / line_height) / 256;
            if (tex_y < 0) {
                tex_y = 0;
            }
            if (tex_y >= TEX_SIZE) {
                tex_y = TEX_SIZE - 1;
            }

            color = texture[tex_y * TEX_SIZE + tex_x];
            if (side == 1 && color > 8) {
                color = (u8)(color - 8);
            }
            framebuffer[y * SCREEN_WIDTH + x] = color;
        }

        g_depth_buffer[x] = perp_wall_dist;
    }

    render_sprites(game, framebuffer);
    draw_status_bar(game, framebuffer);
    draw_crosshair(framebuffer);
}

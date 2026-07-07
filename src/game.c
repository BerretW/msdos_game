#include "game.h"

#include "assets.h"
#include "config.h"
#include "render.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TILE_DEFS 32
#define MAX_ENTITY_DEFS 32
#define MAX_WEAPON_DEFS 16

#define DEFAULT_MAP_WIDTH 64
#define DEFAULT_MAP_HEIGHT 64

#define TILE_KIND_EMPTY 0
#define TILE_KIND_WALL 1
#define TILE_KIND_DOOR 2
#define TILE_KIND_START 3

#define ENTITY_KIND_DECOR 0
#define ENTITY_KIND_ENEMY 1
#define ENTITY_KIND_PICKUP 2
#define ENTITY_KIND_MARKER 3

#define AI_NONE 0
#define AI_MELEE 1
#define AI_RANGED 2

#define KEY_NONE 0
#define KEY_YELLOW 1

#define PLAYER_COLLISION_RADIUS 0.2

typedef struct TileDef {
    int active;
    char symbol;
    int kind;
    int texture_id;
    int key_mask;
} TileDef;

typedef struct EntityDef {
    int active;
    char symbol;
    int kind;
    int texture_id;
    int health;
    int touch_damage;
    int speed_milli;
    int pickup_health;
    int pickup_ammo;
    int pickup_ammo_type_id;
    int pickup_key_mask;
    int pickup_weapon_id;
    int ai_mode;
    int alert_range_milli;
    int attack_range_milli;
    int attack_cooldown_ms;
} EntityDef;

typedef struct WeaponDef {
    int active;
    int id;
    char name[32];
    int ammo_type_id;
    int ammo_cost;
    int damage;
    int range_milli;
    int hit_cone_milli;
    int flash_ms;
    int start_ammo;
    int projectile_speed_milli;
    int projectile_texture_id;
} WeaponDef;

static GameState *g_current_game;
static TileDef g_tile_defs[MAX_TILE_DEFS];
static EntityDef g_entity_defs[MAX_ENTITY_DEFS];
static WeaponDef g_weapon_defs[MAX_WEAPON_DEFS];
static int g_default_start_health = 100;
static int g_default_start_weapon_id = 1;
static int g_defs_loaded;

static const char *g_default_level[DEFAULT_MAP_HEIGHT] = {
    "1111111111111111111111111111111111111111111111111111111111111111",
    "1...............2...............3...............4..............1",
    "1.111111111111..2...44444444444.3.4444444444444.4..............1",
    "1.1..........1..2...4.........4.3.4...........4.4..............1",
    "1.1.B.....H..1..2...4.........4.3.4...........4.4..............1",
    "1.1..........1..2...4.........4.3.4.........H.4.4..............1",
    "1.11111T111111......4...B.....4.3.4...Y.......4.4..............1",
    "1...............2...4.........4.3.4...........4.4..............1",
    "1222.222.222.22222224.........42324...........424222222222222221",
    "1...............2...4.........4.3.4...........4.4..............1",
    "1..11111.11111..2...4......E.............K....4.4..............1",
    "1..1.........1..2...4.M.......4.3.4...........4.4..............1",
    "1..1..E......1..2...4.........4.3.4...........4.4..............1",
    "1..1.........1..2...4.........4.3.4...........4.4..............1",
    "1..1.........1..2...44444444444.3.444444.444444................1",
    "1..1....D....1..2...............3...............4..............1",
    "1..1.........1..2...............3...............4..............1",
    "1..1......M..1..2...............3...............4..............1",
    "1..1............................3...............4.33333333333..1",
    "1..1.........1..2...............3...............4.3.........3..1",
    "1..11111111111..2...............3...............4.3.........3..1",
    "1...............2...............3...............4.3....B....3..1",
    "1...............2...............................4.3...1.1...3..1",
    "1...............2...............3...............4.3.........3..1",
    "12222222222222222222.2222222.22232222222.2222222423.....W...3221",
    "1...............2...............3...............4.3.........3..1",
    "1...............2....2222222222222222222.2222...4.3...1.1...3..1",
    "1...............2...2.......................2...4.3.........3..1",
    "1...............2...2.....................M.2...4.3..E......3..1",
    "1...............2...2...1...1.......1...1...2...4.3.........3..1",
    "1...............2...2...E.................................H.3..1",
    "1...............2...2.......................2...4...........3..1",
    "1...............2...2...........B...........2...4...........3..1",
    "1...............2...2.......................2...4...........3..1",
    "1...................2...................K...2...4..3333333333..1",
    "1...............2...2...1...1.......1...1...2...4..............1",
    "1...............2...2.H.....................2...4..............1",
    "1...............2...2.......................2...4..............1",
    "1...............2...2222222222222222222222222...4..3333333333..1",
    "1...............2...............3...............4.3.........3..1",
    "133333333333333323333333333333333333.3333333.333433.........3331",
    "1...............2...............3...............4.3.......M.3..1",
    "1...............2...............3.4444444444444...3...1.1...3..1",
    "1...............2...............3.4...........4.4.3.........3..1",
    "1...............2...............3.4...........4.4.3.........3..1",
    "1...............2...............3.4........K..4.4.3....K....3..1",
    "1...............................3.4...........4.4.3.........3..1",
    "1...............2...............3.L.....T...................3..1",
    "1...............2...............3.4...........4.4.....1.1...3..1",
    "1...............2...............3.4...H.......4.4...E.......3..1",
    "1...............2.................4...........4.4...........3..1",
    "1...............2...............3.4...........4.4...........3..1",
    "1...............2...............3.4444444444444.4..3.33333333..1",
    "1...............2...............3...............4..............1",
    "14444444444444442444444444444444344444444444444444.4.44444.44441",
    "1...............2...............3...............4..............1",
    "1.11111.1111111.2.1111111111111.3...............4..............1",
    "1.1...........1.2.1...........1.3..............................1",
    "1.1....H...W............B......................................1",
    "1.1.P.........1.2.1..M.....E..1.3...............4..............1",
    "1.1...........1.2.1...........1.3...............4..............1",
    "1.1111111111111.2.1111111111111.3...............4..............1",
    "1...............2...............3...............4..............1",
    "1111111111111111111111111111111111111111111111111111111111111111"
};

static void free_map_tiles(GameState *game)
{
    if (game->map_tiles != 0) {
        free(game->map_tiles);
        game->map_tiles = 0;
    }
    game->map_width = 0;
    game->map_height = 0;
}

static int allocate_map_tiles(GameState *game, int width, int height)
{
    size_t tile_count;

    if (width <= 0 || height <= 0) {
        return 0;
    }

    tile_count = (size_t)width * (size_t)height;
    if (tile_count / (size_t)width != (size_t)height) {
        return 0;
    }

    free_map_tiles(game);
    game->map_tiles = (int *)malloc(tile_count * sizeof(int));
    if (game->map_tiles == 0) {
        return 0;
    }

    game->map_width = width;
    game->map_height = height;
    memset(game->map_tiles, 0, tile_count * sizeof(int));
    return 1;
}

static void set_map_tile(GameState *game, int x, int y, int tile)
{
    if (game->map_tiles == 0) {
        return;
    }
    if (x < 0 || y < 0 || x >= game->map_width || y >= game->map_height) {
        return;
    }
    game->map_tiles[y * game->map_width + x] = tile;
}

static int raw_map_tile_at(const GameState *game, int x, int y)
{
    if (game->map_tiles == 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x >= game->map_width || y >= game->map_height) {
        return 1;
    }
    return game->map_tiles[y * game->map_width + x];
}

static void free_loaded_rows(char **rows, int row_count)
{
    int i;

    if (rows == 0) {
        return;
    }

    for (i = 0; i < row_count; ++i) {
        free(rows[i]);
    }
    free(rows);
}

static int append_loaded_row(char ***rows, int *row_count, int *row_capacity, const char *line, int line_length)
{
    char **new_rows;
    char *row_copy;

    if (*row_count >= *row_capacity) {
        int new_capacity;

        new_capacity = *row_capacity > 0 ? (*row_capacity * 2) : 8;
        new_rows = (char **)realloc(*rows, (size_t)new_capacity * sizeof(char *));
        if (new_rows == 0) {
            return 0;
        }
        *rows = new_rows;
        *row_capacity = new_capacity;
    }

    row_copy = (char *)malloc((size_t)line_length + 1);
    if (row_copy == 0) {
        return 0;
    }

    if (line_length > 0) {
        memcpy(row_copy, line, (size_t)line_length);
    }
    row_copy[line_length] = '\0';
    (*rows)[*row_count] = row_copy;
    ++(*row_count);
    return 1;
}

static int load_level_rows(FILE *file, char ***rows_out, int *width_out, int *height_out)
{
    char **rows;
    char *line_buffer;
    int row_count;
    int row_capacity;
    int line_length;
    int line_capacity;
    int max_width;
    int c;

    rows = 0;
    line_buffer = 0;
    row_count = 0;
    row_capacity = 0;
    line_length = 0;
    line_capacity = 0;
    max_width = 0;

    while ((c = fgetc(file)) != EOF) {
        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            if (!append_loaded_row(&rows, &row_count, &row_capacity, line_buffer, line_length)) {
                free(line_buffer);
                free_loaded_rows(rows, row_count);
                return 0;
            }
            if (line_length > max_width) {
                max_width = line_length;
            }
            line_length = 0;
            continue;
        }

        if (line_length + 1 >= line_capacity) {
            char *new_buffer;
            int new_capacity;

            new_capacity = line_capacity > 0 ? (line_capacity * 2) : 32;
            new_buffer = (char *)realloc(line_buffer, (size_t)new_capacity);
            if (new_buffer == 0) {
                free(line_buffer);
                free_loaded_rows(rows, row_count);
                return 0;
            }
            line_buffer = new_buffer;
            line_capacity = new_capacity;
        }

        line_buffer[line_length++] = (char)c;
    }

    if (line_length > 0 || row_count == 0) {
        if (!append_loaded_row(&rows, &row_count, &row_capacity, line_buffer, line_length)) {
            free(line_buffer);
            free_loaded_rows(rows, row_count);
            return 0;
        }
        if (line_length > max_width) {
            max_width = line_length;
        }
    }

    free(line_buffer);

    if (row_count <= 0 || max_width <= 0) {
        free_loaded_rows(rows, row_count);
        return 0;
    }

    *rows_out = rows;
    *width_out = max_width;
    *height_out = row_count;
    return 1;
}

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

static int parse_named_id(const char *token, char prefix)
{
    if (token == 0 || token[0] != prefix) {
        return -1;
    }
    return atoi(token + 1);
}

static int weapon_mask_for_id(int id)
{
    if (id <= 0 || id > 30) {
        return 0;
    }
    return 1 << (id - 1);
}

static TileDef *find_tile_def(char symbol)
{
    int i;

    for (i = 0; i < MAX_TILE_DEFS; ++i) {
        if (g_tile_defs[i].active && g_tile_defs[i].symbol == symbol) {
            return &g_tile_defs[i];
        }
    }
    return 0;
}

static EntityDef *find_entity_def(char symbol)
{
    int i;

    for (i = 0; i < MAX_ENTITY_DEFS; ++i) {
        if (g_entity_defs[i].active && g_entity_defs[i].symbol == symbol) {
            return &g_entity_defs[i];
        }
    }
    return 0;
}

static WeaponDef *find_weapon_def(int id)
{
    int i;

    for (i = 0; i < MAX_WEAPON_DEFS; ++i) {
        if (g_weapon_defs[i].active && g_weapon_defs[i].id == id) {
            return &g_weapon_defs[i];
        }
    }
    return 0;
}

static void clear_defs(void)
{
    memset(g_tile_defs, 0, sizeof(g_tile_defs));
    memset(g_entity_defs, 0, sizeof(g_entity_defs));
    memset(g_weapon_defs, 0, sizeof(g_weapon_defs));
    g_default_start_health = 100;
    g_default_start_weapon_id = 1;
}

static void add_tile_def(char symbol, int kind, int texture_id, int key_mask)
{
    int i;
    int slot;

    slot = -1;

    for (i = 0; i < MAX_TILE_DEFS; ++i) {
        if (g_tile_defs[i].active && g_tile_defs[i].symbol == symbol) {
            slot = i;
            break;
        }
        if (slot < 0 && !g_tile_defs[i].active) {
            slot = i;
        }
    }

    if (slot >= 0) {
        g_tile_defs[slot].active = 1;
        g_tile_defs[slot].symbol = symbol;
        g_tile_defs[slot].kind = kind;
        g_tile_defs[slot].texture_id = texture_id;
        g_tile_defs[slot].key_mask = key_mask;
    }
}

static void add_entity_def(char symbol, int kind, int texture_id, int health, int touch_damage, int speed_milli, int pickup_health, int pickup_ammo, int pickup_ammo_type_id, int pickup_key_mask, int pickup_weapon_id, int ai_mode, int alert_range_milli, int attack_range_milli, int attack_cooldown_ms)
{
    int i;
    int slot;

    slot = -1;

    for (i = 0; i < MAX_ENTITY_DEFS; ++i) {
        if (g_entity_defs[i].active && g_entity_defs[i].symbol == symbol) {
            slot = i;
            break;
        }
        if (slot < 0 && !g_entity_defs[i].active) {
            slot = i;
        }
    }

    if (slot >= 0) {
        g_entity_defs[slot].active = 1;
        g_entity_defs[slot].symbol = symbol;
        g_entity_defs[slot].kind = kind;
        g_entity_defs[slot].texture_id = texture_id;
        g_entity_defs[slot].health = health;
        g_entity_defs[slot].touch_damage = touch_damage;
        g_entity_defs[slot].speed_milli = speed_milli;
        g_entity_defs[slot].pickup_health = pickup_health;
        g_entity_defs[slot].pickup_ammo = pickup_ammo;
        g_entity_defs[slot].pickup_ammo_type_id = pickup_ammo_type_id;
        g_entity_defs[slot].pickup_key_mask = pickup_key_mask;
        g_entity_defs[slot].pickup_weapon_id = pickup_weapon_id;
        g_entity_defs[slot].ai_mode = ai_mode;
        g_entity_defs[slot].alert_range_milli = alert_range_milli;
        g_entity_defs[slot].attack_range_milli = attack_range_milli;
        g_entity_defs[slot].attack_cooldown_ms = attack_cooldown_ms;
    }
}

static void add_weapon_def(int id, const char *name, int ammo_type_id, int ammo_cost, int damage, int range_milli, int hit_cone_milli, int flash_ms, int start_ammo, int projectile_speed_milli, int projectile_texture_id)
{
    int i;
    int slot;

    slot = -1;

    for (i = 0; i < MAX_WEAPON_DEFS; ++i) {
        if (g_weapon_defs[i].active && g_weapon_defs[i].id == id) {
            slot = i;
            break;
        }
        if (slot < 0 && !g_weapon_defs[i].active) {
            slot = i;
        }
    }

    if (slot >= 0) {
        g_weapon_defs[slot].active = 1;
        g_weapon_defs[slot].id = id;
        strncpy(g_weapon_defs[slot].name, name, sizeof(g_weapon_defs[slot].name) - 1);
        g_weapon_defs[slot].name[sizeof(g_weapon_defs[slot].name) - 1] = '\0';
        g_weapon_defs[slot].ammo_type_id = ammo_type_id;
        g_weapon_defs[slot].ammo_cost = ammo_cost;
        g_weapon_defs[slot].damage = damage;
        g_weapon_defs[slot].range_milli = range_milli;
        g_weapon_defs[slot].hit_cone_milli = hit_cone_milli;
        g_weapon_defs[slot].flash_ms = flash_ms;
        g_weapon_defs[slot].start_ammo = start_ammo;
        g_weapon_defs[slot].projectile_speed_milli = projectile_speed_milli;
        g_weapon_defs[slot].projectile_texture_id = projectile_texture_id;
    }
}

static void load_default_defs(void)
{
    clear_defs();

    add_tile_def('.', TILE_KIND_EMPTY, 0, KEY_NONE);
    add_tile_def('1', TILE_KIND_WALL, 1, KEY_NONE);
    add_tile_def('2', TILE_KIND_WALL, 2, KEY_NONE);
    add_tile_def('3', TILE_KIND_WALL, 3, KEY_NONE);
    add_tile_def('4', TILE_KIND_WALL, 4, KEY_NONE);
    add_tile_def('D', TILE_KIND_DOOR, 7, KEY_NONE);
    add_tile_def('L', TILE_KIND_DOOR, 7, KEY_YELLOW);
    add_tile_def('P', TILE_KIND_START, 0, KEY_NONE);

    add_entity_def('B', ENTITY_KIND_DECOR, 5, 1, 0, 0, 0, 0, 0, 0, 0, AI_NONE, 0, 0, 0);
    add_entity_def('E', ENTITY_KIND_ENEMY, 6, 3, 10, 1100, 0, 0, 0, 0, 0, AI_MELEE, 4800, 1400, 800);
    add_entity_def('K', ENTITY_KIND_ENEMY, 9, 6, 16, 1450, 0, 0, 0, 0, 0, AI_RANGED, 6200, 2400, 1100);
    add_entity_def('H', ENTITY_KIND_PICKUP, 8, 1, 0, 0, 25, 0, 0, 0, 0, AI_NONE, 0, 0, 0);
    add_entity_def('M', ENTITY_KIND_PICKUP, 10, 1, 0, 0, 0, 6, 2, 0, 0, AI_NONE, 0, 0, 0);
    add_entity_def('Y', ENTITY_KIND_PICKUP, 11, 1, 0, 0, 0, 0, 0, KEY_YELLOW, 0, AI_NONE, 0, 0, 0);
    add_entity_def('T', ENTITY_KIND_MARKER, 12, 1, 0, 0, 0, 0, 0, 0, 0, AI_NONE, 0, 0, 0);
    add_entity_def('W', ENTITY_KIND_PICKUP, 14, 1, 0, 0, 0, 6, 2, 0, 2, AI_NONE, 0, 0, 0);

    add_weapon_def(1, "wand", 1, 1, 1, 7500, 450, 90, 12, 0, 0);
    add_weapon_def(2, "crossbow", 2, 2, 3, 9500, 300, 110, 8, 6200, 15);
}

static void load_tiles_file(void)
{
    FILE *file;
    char resolved_path[260];
    char line[256];

    file = open_data_file("assets/tiles.txt", resolved_path);
    if (file == 0) {
        return;
    }

    while (fgets(line, sizeof(line), file) != 0) {
        char symbol;
        char kind_name[32];
        int texture_id;
        int key_mask;
        int kind;

        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        if (sscanf(line, " %c %31s %d %d", &symbol, kind_name, &texture_id, &key_mask) != 4) {
            continue;
        }

        kind = TILE_KIND_EMPTY;
        if (strcmp(kind_name, "EMPTY") == 0) {
            kind = TILE_KIND_EMPTY;
        } else if (strcmp(kind_name, "WALL") == 0) {
            kind = TILE_KIND_WALL;
        } else if (strcmp(kind_name, "DOOR") == 0) {
            kind = TILE_KIND_DOOR;
        } else if (strcmp(kind_name, "START") == 0) {
            kind = TILE_KIND_START;
        }

        add_tile_def(symbol, kind, texture_id, key_mask);
    }

    fclose(file);
}

static void load_entities_file(void)
{
    FILE *file;
    char resolved_path[260];
    char line[256];

    file = open_data_file("assets/entities.txt", resolved_path);
    if (file == 0) {
        return;
    }

    while (fgets(line, sizeof(line), file) != 0) {
        char symbol;
        char kind_name[32];
        char ai_name[32];
        int texture_id;
        int health;
        int touch_damage;
        int speed_milli;
        int pickup_health;
        int pickup_ammo;
        int pickup_ammo_type_id;
        int pickup_key_mask;
        int pickup_weapon_id;
        int alert_range_milli;
        int attack_range_milli;
        int attack_cooldown_ms;
        int kind;
        int ai_mode;

        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        if (sscanf(line, " %c %31s %d %d %d %d %d %d %d %d %d %31s %d %d %d", &symbol, kind_name, &texture_id, &health, &touch_damage, &speed_milli, &pickup_health, &pickup_ammo, &pickup_ammo_type_id, &pickup_key_mask, &pickup_weapon_id, ai_name, &alert_range_milli, &attack_range_milli, &attack_cooldown_ms) != 15) {
            continue;
        }

        kind = ENTITY_KIND_DECOR;
        if (strcmp(kind_name, "DECOR") == 0) {
            kind = ENTITY_KIND_DECOR;
        } else if (strcmp(kind_name, "ENEMY") == 0) {
            kind = ENTITY_KIND_ENEMY;
        } else if (strcmp(kind_name, "PICKUP") == 0) {
            kind = ENTITY_KIND_PICKUP;
        } else if (strcmp(kind_name, "MARKER") == 0) {
            kind = ENTITY_KIND_MARKER;
        }

        ai_mode = AI_NONE;
        if (strcmp(ai_name, "MELEE") == 0) {
            ai_mode = AI_MELEE;
        } else if (strcmp(ai_name, "RANGED") == 0) {
            ai_mode = AI_RANGED;
        }

        add_entity_def(symbol, kind, texture_id, health, touch_damage, speed_milli, pickup_health, pickup_ammo, pickup_ammo_type_id, pickup_key_mask, pickup_weapon_id, ai_mode, alert_range_milli, attack_range_milli, attack_cooldown_ms);
    }

    fclose(file);
}

static void load_weapons_file(void)
{
    FILE *file;
    char resolved_path[260];
    char line[256];

    file = open_data_file("assets/weapons.txt", resolved_path);
    if (file == 0) {
        return;
    }

    while (fgets(line, sizeof(line), file) != 0) {
        char name[32];
        int id;
        int ammo_type_id;
        int ammo_cost;
        int damage;
        int range_milli;
        int hit_cone_milli;
        int flash_ms;
        int start_ammo;
        int projectile_speed_milli;
        int projectile_texture_id;

        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        if (sscanf(line, " %d %31s %d %d %d %d %d %d %d %d %d", &id, name, &ammo_type_id, &ammo_cost, &damage, &range_milli, &hit_cone_milli, &flash_ms, &start_ammo, &projectile_speed_milli, &projectile_texture_id) != 11) {
            continue;
        }

        add_weapon_def(id, name, ammo_type_id, ammo_cost, damage, range_milli, hit_cone_milli, flash_ms, start_ammo, projectile_speed_milli, projectile_texture_id);
    }

    fclose(file);
}

static void load_game_file(void)
{
    FILE *file;
    char resolved_path[260];
    char key[64];
    char value[64];

    file = open_data_file("assets/game.txt", resolved_path);
    if (file == 0) {
        return;
    }

    while (fscanf(file, " %63s %63s", key, value) == 2) {
        if (key[0] == '#') {
            fgets(value, sizeof(value), file);
            continue;
        }
        if (strcmp(key, "start_health") == 0) {
            g_default_start_health = atoi(value);
        } else if (strcmp(key, "start_weapon") == 0) {
            g_default_start_weapon_id = atoi(value);
        }
    }

    fclose(file);
}

static int weapon_is_owned(const GameState *game, int weapon_id)
{
    int mask;

    mask = weapon_mask_for_id(weapon_id);
    return mask != 0 && (game->weapon_owned_mask & mask) != 0;
}

static int ammo_type_is_valid(int ammo_type_id)
{
    return ammo_type_id > 0 && ammo_type_id < MAX_AMMO_TYPES;
}

static int ammo_get(const GameState *game, int ammo_type_id)
{
    if (!ammo_type_is_valid(ammo_type_id)) {
        return 0;
    }
    return game->ammo_by_type[ammo_type_id];
}

static void ammo_add(GameState *game, int ammo_type_id, int amount)
{
    if (!ammo_type_is_valid(ammo_type_id) || amount <= 0) {
        return;
    }
    game->ammo_by_type[ammo_type_id] += amount;
    if (game->ammo_by_type[ammo_type_id] > 99) {
        game->ammo_by_type[ammo_type_id] = 99;
    }
}

static int ammo_consume(GameState *game, int ammo_type_id, int amount)
{
    if (!ammo_type_is_valid(ammo_type_id)) {
        return amount <= 0 ? 1 : 0;
    }
    if (game->ammo_by_type[ammo_type_id] < amount) {
        return 0;
    }
    game->ammo_by_type[ammo_type_id] -= amount;
    return 1;
}

static int current_weapon_ammo_type(const GameState *game)
{
    WeaponDef *weapon;

    weapon = find_weapon_def(game->current_weapon_id);
    if (weapon == 0) {
        return 0;
    }
    return weapon->ammo_type_id;
}

static int current_weapon_ammo(const GameState *game)
{
    return ammo_get(game, current_weapon_ammo_type(game));
}

static SpriteState *alloc_runtime_sprite(GameState *game)
{
    int i;

    for (i = 0; i < game->sprite_count; ++i) {
        if (!game->sprites[i].active) {
            memset(&game->sprites[i], 0, sizeof(game->sprites[i]));
            game->sprites[i].active = 1;
            return &game->sprites[i];
        }
    }

    if (game->sprite_count >= MAX_SPRITES) {
        return 0;
    }

    memset(&game->sprites[game->sprite_count], 0, sizeof(game->sprites[game->sprite_count]));
    game->sprites[game->sprite_count].active = 1;
    game->sprite_count += 1;
    return &game->sprites[game->sprite_count - 1];
}

static SpriteState *spawn_projectile(GameState *game, double x, double y, double dir_x, double dir_y, int texture_id, int damage, int speed_milli, int ttl_ms, int owner)
{
    SpriteState *sprite;
    double length;

    sprite = alloc_runtime_sprite(game);
    if (sprite == 0) {
        return 0;
    }

    length = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (length < 0.0001) {
        sprite->active = 0;
        return 0;
    }

    sprite->x = x;
    sprite->y = y;
    sprite->vel_x = (dir_x / length) * ((double)speed_milli / 1000.0);
    sprite->vel_y = (dir_y / length) * ((double)speed_milli / 1000.0);
    sprite->texture_id = texture_id;
    sprite->is_projectile = 1;
    sprite->projectile_owner = owner;
    sprite->projectile_damage = damage;
    sprite->projectile_ttl_ms = ttl_ms;
    return sprite;
}

static void grant_weapon(GameState *game, int weapon_id, int auto_select)
{
    int mask;
    WeaponDef *weapon;

    mask = weapon_mask_for_id(weapon_id);
    weapon = find_weapon_def(weapon_id);
    if (mask == 0 || weapon == 0) {
        return;
    }

    game->weapon_owned_mask |= mask;
    ammo_add(game, weapon->ammo_type_id, weapon->start_ammo);
    if (auto_select || !weapon_is_owned(game, game->current_weapon_id)) {
        game->current_weapon_id = weapon_id;
    }
}

static void select_next_weapon(GameState *game)
{
    int i;
    int first_id;
    int next_id;

    first_id = 0;
    next_id = 0;
    for (i = 0; i < MAX_WEAPON_DEFS; ++i) {
        int weapon_id;

        if (!g_weapon_defs[i].active) {
            continue;
        }
        weapon_id = g_weapon_defs[i].id;
        if (!weapon_is_owned(game, weapon_id)) {
            continue;
        }
        if (first_id == 0 || weapon_id < first_id) {
            first_id = weapon_id;
        }
        if (weapon_id > game->current_weapon_id && (next_id == 0 || weapon_id < next_id)) {
            next_id = weapon_id;
        }
    }

    if (next_id != 0) {
        game->current_weapon_id = next_id;
    } else if (first_id != 0) {
        game->current_weapon_id = first_id;
    }
}

static void ensure_defs_loaded(void)
{
    if (g_defs_loaded) {
        return;
    }
    load_default_defs();
    load_tiles_file();
    load_entities_file();
    load_weapons_file();
    load_game_file();
    g_defs_loaded = 1;
}

static void reset_runtime_arrays(GameState *game)
{
    int i;

    game->sprite_count = 0;
    game->door_count = 0;
    game->trigger_count = 0;
    game->actuator_count = 0;
    game->pending_map_change = 0;

    game->player_x = 2.5;
    game->player_y = 13.5;
    game->dir_x = 1.0;
    game->dir_y = 0.0;
    game->plane_x = 0.0;
    game->plane_y = 0.66;

    for (i = 0; i < MAX_SPRITES; ++i) {
        memset(&game->sprites[i], 0, sizeof(game->sprites[i]));
    }
    for (i = 0; i < MAX_DOORS; ++i) {
        game->doors[i].map_x = -1;
        game->doors[i].map_y = -1;
        game->doors[i].openness = 0.0;
        game->doors[i].opening = 0;
        game->doors[i].hold_open_ms = 0;
        game->doors[i].required_key_mask = 0;
    }
    for (i = 0; i < MAX_TRIGGERS; ++i) {
        memset(&game->triggers[i], 0, sizeof(game->triggers[i]));
    }
    for (i = 0; i < MAX_ACTUATORS; ++i) {
        memset(&game->actuators[i], 0, sizeof(game->actuators[i]));
    }
}

static DoorState *add_door(GameState *game, int x, int y, int required_key_mask)
{
    DoorState *door;

    if (game->door_count >= MAX_DOORS) {
        return 0;
    }

    door = &game->doors[game->door_count++];
    door->map_x = x;
    door->map_y = y;
    door->openness = 0.0;
    door->opening = 0;
    door->hold_open_ms = 0;
    door->required_key_mask = required_key_mask;
    return door;
}

static SpriteState *spawn_symbol(GameState *game, int x, int y, char cell)
{
    EntityDef *def;
    SpriteState *sprite;

    def = find_entity_def(cell);
    if (def == 0) {
        return 0;
    }

    sprite = alloc_runtime_sprite(game);
    if (sprite == 0) {
        return 0;
    }
    sprite->x = (double)x + 0.5;
    sprite->y = (double)y + 0.5;
    sprite->texture_id = def->texture_id;
    sprite->health = def->health;
    sprite->touch_damage = def->touch_damage;
    sprite->speed_milli = def->speed_milli;
    sprite->pickup_health = def->pickup_health;
    sprite->pickup_ammo = def->pickup_ammo;
    sprite->pickup_ammo_type_id = def->pickup_ammo_type_id;
    sprite->pickup_key_mask = def->pickup_key_mask;
    sprite->pickup_weapon_id = def->pickup_weapon_id;
    sprite->is_enemy = def->kind == ENTITY_KIND_ENEMY ? 1 : 0;
    sprite->ai_mode = def->ai_mode;
    sprite->alert_range_milli = def->alert_range_milli;
    sprite->attack_range_milli = def->attack_range_milli;
    sprite->attack_cooldown_ms = def->attack_cooldown_ms;
    return sprite;
}

static void apply_level_cell(GameState *game, int x, int y, char cell)
{
    TileDef *tile_def;

    tile_def = find_tile_def(cell);
    if (tile_def != 0) {
        if (tile_def->kind == TILE_KIND_EMPTY) {
            set_map_tile(game, x, y, TILE_EMPTY);
        } else if (tile_def->kind == TILE_KIND_WALL) {
            set_map_tile(game, x, y, tile_def->texture_id);
        } else if (tile_def->kind == TILE_KIND_DOOR) {
            add_door(game, x, y, tile_def->key_mask);
            set_map_tile(game, x, y, TILE_DOOR);
        } else if (tile_def->kind == TILE_KIND_START) {
            game->player_x = (double)x + 0.5;
            game->player_y = (double)y + 0.5;
        }
        return;
    }

    if (find_entity_def(cell) != 0) {
        spawn_symbol(game, x, y, cell);
    }
}

static DoorState *find_door(GameState *game, int map_x, int map_y)
{
    int i;

    for (i = 0; i < game->door_count; ++i) {
        if (game->doors[i].map_x == map_x && game->doors[i].map_y == map_y) {
            return &game->doors[i];
        }
    }

    return 0;
}

static const DoorState *find_const_door(const GameState *game, int map_x, int map_y)
{
    int i;

    for (i = 0; i < game->door_count; ++i) {
        if (game->doors[i].map_x == map_x && game->doors[i].map_y == map_y) {
            return &game->doors[i];
        }
    }

    return 0;
}

static void set_default_map(GameState *game)
{
    int x;
    int y;

    if (!allocate_map_tiles(game, DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT)) {
        free_map_tiles(game);
        return;
    }

    reset_runtime_arrays(game);
    for (y = 0; y < DEFAULT_MAP_HEIGHT; ++y) {
        for (x = 0; x < DEFAULT_MAP_WIDTH; ++x) {
            apply_level_cell(game, x, y, g_default_level[y][x]);
        }
    }
}

static void load_level_file_named(GameState *game, const char *path)
{
    FILE *file;
    char **rows;
    char resolved_path[260];
    int width;
    int height;
    int x;
    int y;

    file = open_data_file(path, resolved_path);
    if (file == 0) {
        set_default_map(game);
        return;
    }

    rows = 0;
    width = 0;
    height = 0;
    if (!load_level_rows(file, &rows, &width, &height)) {
        fclose(file);
        set_default_map(game);
        return;
    }
    fclose(file);

    if (!allocate_map_tiles(game, width, height)) {
        free_loaded_rows(rows, height);
        set_default_map(game);
        return;
    }

    reset_runtime_arrays(game);
    for (y = 0; y < height; ++y) {
        int row_length;

        row_length = (int)strlen(rows[y]);
        for (x = 0; x < width; ++x) {
            char cell;

            cell = x < row_length ? rows[y][x] : '.';
            apply_level_cell(game, x, y, cell);
        }
    }

    free_loaded_rows(rows, height);
}

static void add_trigger(GameState *game, int id, int mode, int map_x, int map_y, int target_id, int once)
{
    TriggerState *trigger;

    if (game->trigger_count >= MAX_TRIGGERS) {
        return;
    }

    trigger = &game->triggers[game->trigger_count++];
    memset(trigger, 0, sizeof(*trigger));
    trigger->active = 1;
    trigger->id = id;
    trigger->mode = mode;
    trigger->map_x = map_x;
    trigger->map_y = map_y;
    trigger->target_id = target_id;
    trigger->once = once;
}

static void add_actuator(GameState *game, int target_id, int type)
{
    ActuatorState *actuator;

    if (game->actuator_count >= MAX_ACTUATORS) {
        return;
    }

    actuator = &game->actuators[game->actuator_count++];
    memset(actuator, 0, sizeof(*actuator));
    actuator->active = 1;
    actuator->target_id = target_id;
    actuator->type = type;
}

static void load_logic_file_named(GameState *game, const char *path)
{
    FILE *file;
    char resolved_path[260];
    char line[256];

    file = open_data_file(path, resolved_path);
    if (file == 0) {
        return;
    }

    while (fgets(line, sizeof(line), file) != 0) {
        char *tokens[8];
        int count;
        char *token;

        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        count = 0;
        token = strtok(line, " \t\r\n");
        while (token != 0 && count < 8) {
            tokens[count++] = token;
            token = strtok(0, " \t\r\n");
        }
        if (count == 0) {
            continue;
        }

        if (strcmp(tokens[0], "TRIGGER") == 0 && count >= 6) {
            int id;
            int mode;
            int target_id;
            int once;

            id = parse_named_id(tokens[1], 'T');
            mode = 0;
            if (strcmp(tokens[2], "USE") == 0) {
                mode = TRIGGER_MODE_USE;
            } else if (strcmp(tokens[2], "ENTER") == 0) {
                mode = TRIGGER_MODE_ENTER;
            }
            target_id = parse_named_id(tokens[5], 'A');
            once = (count >= 7) ? atoi(tokens[6]) : 0;
            if (id >= 0 && target_id >= 0 && mode != 0) {
                add_trigger(game, id, mode, atoi(tokens[3]), atoi(tokens[4]), target_id, once);
            }
        } else if (strcmp(tokens[0], "ACTUATOR") == 0 && count >= 3) {
            int target_id;

            target_id = parse_named_id(tokens[1], 'A');
            if (target_id < 0) {
                continue;
            }

            if (strcmp(tokens[2], "DOOR") == 0 && count >= 5) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_OPEN_DOOR);
                actuator = &game->actuators[game->actuator_count - 1];
                actuator->map_x = atoi(tokens[3]);
                actuator->map_y = atoi(tokens[4]);
                actuator->value = (count >= 6) ? atoi(tokens[5]) : 1800;
            } else if (strcmp(tokens[2], "SPAWN") == 0 && count >= 6) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_SPAWN);
                actuator = &game->actuators[game->actuator_count - 1];
                actuator->map_x = atoi(tokens[3]);
                actuator->map_y = atoi(tokens[4]);
                actuator->symbol = tokens[5][0];
            } else if (strcmp(tokens[2], "MAP") == 0 && count >= 4) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_MAP);
                actuator = &game->actuators[game->actuator_count - 1];
                strncpy(actuator->text_arg0, tokens[3], sizeof(actuator->text_arg0) - 1);
                if (count >= 5) {
                    strncpy(actuator->text_arg1, tokens[4], sizeof(actuator->text_arg1) - 1);
                }
            } else if (strcmp(tokens[2], "HEAL") == 0 && count >= 4) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_HEAL);
                actuator = &game->actuators[game->actuator_count - 1];
                actuator->value = atoi(tokens[3]);
            } else if (strcmp(tokens[2], "AMMO") == 0 && count >= 4) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_GIVE_AMMO);
                actuator = &game->actuators[game->actuator_count - 1];
                if (count >= 5) {
                    actuator->value = atoi(tokens[3]);
                    actuator->value1 = atoi(tokens[4]);
                } else {
                    actuator->value = 1;
                    actuator->value1 = atoi(tokens[3]);
                }
            } else if (strcmp(tokens[2], "KEY") == 0 && count >= 4) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_GIVE_KEY);
                actuator = &game->actuators[game->actuator_count - 1];
                actuator->value = atoi(tokens[3]);
            } else if ((strcmp(tokens[2], "WEAPON") == 0 || strcmp(tokens[2], "GIVE_WEAPON") == 0) && count >= 4) {
                ActuatorState *actuator;

                add_actuator(game, target_id, ACTUATOR_GIVE_WEAPON);
                actuator = &game->actuators[game->actuator_count - 1];
                actuator->value = atoi(tokens[3]);
            }
        }
    }

    fclose(file);
}

static void load_scenario(GameState *game, const char *level_path, const char *logic_path, int preserve_inventory)
{
    int health;
    int ammo_by_type[MAX_AMMO_TYPES];
    int keys_mask;
    int current_weapon_id;
    int weapon_owned_mask;

    health = game->health;
    memcpy(ammo_by_type, game->ammo_by_type, sizeof(ammo_by_type));
    keys_mask = game->keys_mask;
    current_weapon_id = game->current_weapon_id;
    weapon_owned_mask = game->weapon_owned_mask;

    load_level_file_named(game, level_path);
    load_logic_file_named(game, logic_path);

    strncpy(game->current_level_path, level_path, sizeof(game->current_level_path) - 1);
    game->current_level_path[sizeof(game->current_level_path) - 1] = '\0';
    strncpy(game->current_logic_path, logic_path, sizeof(game->current_logic_path) - 1);
    game->current_logic_path[sizeof(game->current_logic_path) - 1] = '\0';

    if (preserve_inventory) {
        game->health = health;
        memcpy(game->ammo_by_type, ammo_by_type, sizeof(game->ammo_by_type));
        game->keys_mask = keys_mask;
        game->current_weapon_id = current_weapon_id;
        game->weapon_owned_mask = weapon_owned_mask;
    } else {
        game->health = g_default_start_health;
        game->current_weapon_id = g_default_start_weapon_id;
        memset(game->ammo_by_type, 0, sizeof(game->ammo_by_type));
        game->keys_mask = 0;
        game->weapon_owned_mask = 0;
        grant_weapon(game, game->current_weapon_id, 1);
    }

    game->weapon_flash_ms = 0;
    game->damage_flash_ms = 0;
    game->use_was_down = 0;
    game->fire_was_down = 0;
    game->next_weapon_was_down = 0;
    game->pending_map_change = 0;
    game->next_level_path[0] = '\0';
    game->next_logic_path[0] = '\0';
    g_current_game = game;
}

static int is_blocked(double x, double y)
{
    int map_x;
    int map_y;

    map_x = (int)x;
    map_y = (int)y;
    return game_map_at(map_x, map_y) != TILE_EMPTY;
}

static int can_move_player_to(double x, double y)
{
    if (is_blocked(x - PLAYER_COLLISION_RADIUS, y - PLAYER_COLLISION_RADIUS)) {
        return 0;
    }
    if (is_blocked(x + PLAYER_COLLISION_RADIUS, y - PLAYER_COLLISION_RADIUS)) {
        return 0;
    }
    if (is_blocked(x - PLAYER_COLLISION_RADIUS, y + PLAYER_COLLISION_RADIUS)) {
        return 0;
    }
    if (is_blocked(x + PLAYER_COLLISION_RADIUS, y + PLAYER_COLLISION_RADIUS)) {
        return 0;
    }
    return 1;
}

static int line_hits_wall(const GameState *game, double x0, double y0, double x1, double y1)
{
    double dx;
    double dy;
    double distance;
    int steps;
    int i;

    (void)game;
    dx = x1 - x0;
    dy = y1 - y0;
    distance = sqrt(dx * dx + dy * dy);
    steps = (int)(distance / 0.08);
    if (steps < 1) {
        steps = 1;
    }

    for (i = 1; i <= steps; ++i) {
        int map_x;
        int map_y;
        double sample_x;
        double sample_y;

        sample_x = x0 + dx * ((double)i / (double)steps);
        sample_y = y0 + dy * ((double)i / (double)steps);
        map_x = (int)sample_x;
        map_y = (int)sample_y;
        if (game_map_at(map_x, map_y) != TILE_EMPTY) {
            return 1;
        }
    }

    return 0;
}

static void open_door_state(DoorState *door, int hold_open_ms)
{
    if (door == 0) {
        return;
    }
    door->opening = 1;
    door->hold_open_ms = hold_open_ms;
}

static void execute_actuator(GameState *game, ActuatorState *actuator)
{
    if (actuator->type == ACTUATOR_OPEN_DOOR) {
        DoorState *door;

        door = find_door(game, actuator->map_x, actuator->map_y);
        if (door != 0) {
            open_door_state(door, actuator->value > 0 ? actuator->value : 1800);
        }
    } else if (actuator->type == ACTUATOR_SPAWN) {
        if (game_map_at(actuator->map_x, actuator->map_y) == TILE_EMPTY) {
            spawn_symbol(game, actuator->map_x, actuator->map_y, actuator->symbol);
        }
    } else if (actuator->type == ACTUATOR_MAP) {
        game->pending_map_change = 1;
        strncpy(game->next_level_path, actuator->text_arg0, sizeof(game->next_level_path) - 1);
        game->next_level_path[sizeof(game->next_level_path) - 1] = '\0';
        if (actuator->text_arg1[0] != '\0') {
            strncpy(game->next_logic_path, actuator->text_arg1, sizeof(game->next_logic_path) - 1);
            game->next_logic_path[sizeof(game->next_logic_path) - 1] = '\0';
        } else {
            strncpy(game->next_logic_path, game->current_logic_path, sizeof(game->next_logic_path) - 1);
            game->next_logic_path[sizeof(game->next_logic_path) - 1] = '\0';
        }
    } else if (actuator->type == ACTUATOR_HEAL) {
        game->health += actuator->value;
        if (game->health > 100) {
            game->health = 100;
        }
    } else if (actuator->type == ACTUATOR_GIVE_AMMO) {
        ammo_add(game, actuator->value, actuator->value1);
    } else if (actuator->type == ACTUATOR_GIVE_KEY) {
        game->keys_mask |= actuator->value;
    } else if (actuator->type == ACTUATOR_GIVE_WEAPON) {
        grant_weapon(game, actuator->value, 1);
    }
}

static void fire_trigger(GameState *game, TriggerState *trigger)
{
    int i;

    if (!trigger->active) {
        return;
    }
    if (trigger->once && trigger->fired) {
        return;
    }

    for (i = 0; i < game->actuator_count; ++i) {
        if (game->actuators[i].active && game->actuators[i].target_id == trigger->target_id) {
            execute_actuator(game, &game->actuators[i]);
        }
    }

    trigger->fired = 1;
}

static int try_activate_use_trigger(GameState *game)
{
    int i;

    for (i = 0; i < game->trigger_count; ++i) {
        TriggerState *trigger;
        double dx;
        double dy;
        double distance;
        double forward;

        trigger = &game->triggers[i];
        if (!trigger->active || trigger->mode != TRIGGER_MODE_USE) {
            continue;
        }
        if (trigger->once && trigger->fired) {
            continue;
        }

        dx = ((double)trigger->map_x + 0.5) - game->player_x;
        dy = ((double)trigger->map_y + 0.5) - game->player_y;
        distance = sqrt(dx * dx + dy * dy);
        forward = dx * game->dir_x + dy * game->dir_y;
        if (distance <= 1.7 && forward > 0.2) {
            fire_trigger(game, trigger);
            return 1;
        }
    }

    return 0;
}

static void update_enter_triggers(GameState *game)
{
    int i;

    for (i = 0; i < game->trigger_count; ++i) {
        TriggerState *trigger;
        int inside;

        trigger = &game->triggers[i];
        if (!trigger->active || trigger->mode != TRIGGER_MODE_ENTER) {
            continue;
        }

        inside = ((int)game->player_x == trigger->map_x && (int)game->player_y == trigger->map_y) ? 1 : 0;
        if (inside && !trigger->was_inside) {
            fire_trigger(game, trigger);
        }
        trigger->was_inside = inside;
    }
}

static void try_open_door(GameState *game)
{
    double target_x;
    double target_y;
    int map_x;
    int map_y;
    DoorState *door;

    target_x = game->player_x + game->dir_x * 1.25;
    target_y = game->player_y + game->dir_y * 1.25;
    map_x = (int)target_x;
    map_y = (int)target_y;
    door = find_door(game, map_x, map_y);
    if (door == 0) {
        return;
    }

    if (door->required_key_mask != 0 && (game->keys_mask & door->required_key_mask) != door->required_key_mask) {
        game->damage_flash_ms = 60;
        return;
    }

    open_door_state(door, 1800);
}

static void try_fire_weapon(GameState *game)
{
    WeaponDef *weapon;

    weapon = find_weapon_def(game->current_weapon_id);
    if (weapon == 0) {
        return;
    }

    if (current_weapon_ammo(game) < weapon->ammo_cost) {
        game->weapon_flash_ms = weapon->flash_ms / 3;
        return;
    }

    if (!ammo_consume(game, weapon->ammo_type_id, weapon->ammo_cost)) {
        game->weapon_flash_ms = weapon->flash_ms / 3;
        return;
    }
    game->weapon_flash_ms = weapon->flash_ms;
    if (weapon->projectile_speed_milli > 0 && weapon->projectile_texture_id > 0) {
        spawn_projectile(
            game,
            game->player_x + game->dir_x * 0.35,
            game->player_y + game->dir_y * 0.35,
            game->dir_x,
            game->dir_y,
            weapon->projectile_texture_id,
            weapon->damage,
            weapon->projectile_speed_milli,
            weapon->range_milli,
            1);
    } else {
        int i;
        int best_index;
        double best_distance;
        double range;
        double cone;

        best_index = -1;
        best_distance = 1e30;
        range = (double)weapon->range_milli / 1000.0;
        cone = (double)weapon->hit_cone_milli / 1000.0;

        for (i = 0; i < game->sprite_count; ++i) {
            SpriteState *sprite;
            double rel_x;
            double rel_y;
            double forward;
            double side;
            double distance;

            sprite = &game->sprites[i];
            if (!sprite->active || !sprite->is_enemy || sprite->is_projectile) {
                continue;
            }

            rel_x = sprite->x - game->player_x;
            rel_y = sprite->y - game->player_y;
            distance = sqrt(rel_x * rel_x + rel_y * rel_y);
            if (distance > range) {
                continue;
            }

            forward = rel_x * game->dir_x + rel_y * game->dir_y;
            side = rel_x * (-game->dir_y) + rel_y * game->dir_x;
            if (forward <= 0.0) {
                continue;
            }
            if (side < -cone || side > cone) {
                continue;
            }
            if (line_hits_wall(game, game->player_x, game->player_y, sprite->x, sprite->y)) {
                continue;
            }
            if (distance < best_distance) {
                best_distance = distance;
                best_index = i;
            }
        }

        if (best_index >= 0) {
            SpriteState *target;

            target = &game->sprites[best_index];
            target->health -= weapon->damage;
            if (target->health <= 0) {
                target->active = 0;
            }
        }
    }
}

static void update_projectiles(GameState *game, u32 delta_ms)
{
    int i;

    for (i = 0; i < game->sprite_count; ++i) {
        SpriteState *sprite;
        double total_step;
        int steps;
        int step_index;

        sprite = &game->sprites[i];
        if (!sprite->active || !sprite->is_projectile) {
            continue;
        }

        sprite->projectile_ttl_ms -= (int)delta_ms;
        if (sprite->projectile_ttl_ms <= 0) {
            sprite->active = 0;
            continue;
        }

        total_step = sqrt(sprite->vel_x * sprite->vel_x + sprite->vel_y * sprite->vel_y) * ((double)delta_ms / 1000.0);
        steps = (int)(total_step / 0.08);
        if (steps < 1) {
            steps = 1;
        }

        for (step_index = 0; step_index < steps && sprite->active; ++step_index) {
            double next_x;
            double next_y;
            int target_index;

            next_x = sprite->x + sprite->vel_x * ((double)delta_ms / 1000.0) / (double)steps;
            next_y = sprite->y + sprite->vel_y * ((double)delta_ms / 1000.0) / (double)steps;
            if (game_map_at((int)next_x, (int)next_y) != TILE_EMPTY) {
                sprite->active = 0;
                break;
            }

            sprite->x = next_x;
            sprite->y = next_y;

            if (sprite->projectile_owner == 1) {
                for (target_index = 0; target_index < game->sprite_count; ++target_index) {
                    SpriteState *target;
                    double dx;
                    double dy;

                    target = &game->sprites[target_index];
                    if (!target->active || !target->is_enemy || target->is_projectile) {
                        continue;
                    }
                    dx = target->x - sprite->x;
                    dy = target->y - sprite->y;
                    if ((dx * dx + dy * dy) < 0.18) {
                        target->health -= sprite->projectile_damage;
                        if (target->health <= 0) {
                            target->active = 0;
                        }
                        sprite->active = 0;
                        break;
                    }
                }
            }
        }
    }
}

static void update_doors(GameState *game, u32 delta_ms)
{
    int i;
    double door_speed;

    door_speed = (double)delta_ms / 700.0;
    for (i = 0; i < game->door_count; ++i) {
        DoorState *door;
        double center_x;
        double center_y;
        double dx;
        double dy;
        double distance;

        door = &game->doors[i];
        center_x = (double)door->map_x + 0.5;
        center_y = (double)door->map_y + 0.5;
        dx = center_x - game->player_x;
        dy = center_y - game->player_y;
        distance = sqrt(dx * dx + dy * dy);

        if (door->opening) {
            door->openness += door_speed;
            if (door->openness > 1.0) {
                door->openness = 1.0;
            }
            if (door->hold_open_ms > 0) {
                door->hold_open_ms -= (int)delta_ms;
            }
            if (door->hold_open_ms <= 0 && distance > 1.25) {
                door->opening = 0;
            }
        } else if (distance > 0.9) {
            door->openness -= door_speed;
            if (door->openness < 0.0) {
                door->openness = 0.0;
            }
        }
    }
}

static void update_enemies(GameState *game, u32 delta_ms)
{
    int i;

    for (i = 0; i < game->sprite_count; ++i) {
        SpriteState *sprite;

        sprite = &game->sprites[i];
        if (!sprite->active || !sprite->is_enemy || sprite->is_projectile) {
            continue;
        }

        if (sprite->cooldown_ms > 0) {
            sprite->cooldown_ms -= (int)delta_ms;
            if (sprite->cooldown_ms < 0) {
                sprite->cooldown_ms = 0;
            }
        }

        {
            double dx;
            double dy;
            double distance;
            double move_speed;
            double alert_range;
            double attack_range;
            int can_see_player;
            int attack_cooldown_ms;

            dx = game->player_x - sprite->x;
            dy = game->player_y - sprite->y;
            distance = sqrt(dx * dx + dy * dy);
            move_speed = (double)delta_ms / 1000.0 * ((double)sprite->speed_milli / 1000.0);

            alert_range = sprite->alert_range_milli > 0 ? (double)sprite->alert_range_milli / 1000.0 : 4.8;
            attack_range = sprite->attack_range_milli > 0 ? (double)sprite->attack_range_milli / 1000.0 : 1.4;
            attack_cooldown_ms = sprite->attack_cooldown_ms > 0 ? sprite->attack_cooldown_ms : 800;
            can_see_player = !line_hits_wall(game, sprite->x, sprite->y, game->player_x, game->player_y);

            if (distance > 0.001 && distance < alert_range && can_see_player) {
                if (distance > attack_range) {
                    double step_x;
                    double step_y;

                    if (sprite->ai_mode == AI_NONE) {
                        continue;
                    }
                    step_x = dx / distance * move_speed;
                    step_y = dy / distance * move_speed;
                    if (!is_blocked(sprite->x + step_x, sprite->y)) {
                        sprite->x += step_x;
                    }
                    if (!is_blocked(sprite->x, sprite->y + step_y)) {
                        sprite->y += step_y;
                    }
                } else if (sprite->cooldown_ms == 0) {
                    game->health -= sprite->touch_damage;
                    if (game->health < 0) {
                        game->health = 0;
                    }
                    game->damage_flash_ms = 120;
                    sprite->cooldown_ms = attack_cooldown_ms;
                }
            }
        }
    }
}

static void update_pickups(GameState *game)
{
    int i;

    for (i = 0; i < game->sprite_count; ++i) {
        SpriteState *sprite;
        double dx;
        double dy;

        sprite = &game->sprites[i];
        if (!sprite->active || sprite->is_enemy || sprite->is_projectile) {
            continue;
        }

        dx = sprite->x - game->player_x;
        dy = sprite->y - game->player_y;
        if ((dx * dx + dy * dy) < 0.30) {
            int consumed;

            consumed = 0;
            if (sprite->pickup_health > 0) {
                game->health += sprite->pickup_health;
                if (game->health > 100) {
                    game->health = 100;
                }
                consumed = 1;
            }
            if (sprite->pickup_ammo > 0) {
                ammo_add(game, sprite->pickup_ammo_type_id, sprite->pickup_ammo);
                consumed = 1;
            }
            if (sprite->pickup_key_mask > 0) {
                game->keys_mask |= sprite->pickup_key_mask;
                consumed = 1;
            }
            if (sprite->pickup_weapon_id > 0) {
                grant_weapon(game, sprite->pickup_weapon_id, 1);
                consumed = 1;
            }
            if (consumed) {
                sprite->active = 0;
            }
        }
    }
}

static void rotate_player(GameState *game, double angle)
{
    double old_dir_x;
    double old_plane_x;

    old_dir_x = game->dir_x;
    old_plane_x = game->plane_x;
    game->dir_x = game->dir_x * cos(angle) - game->dir_y * sin(angle);
    game->dir_y = old_dir_x * sin(angle) + game->dir_y * cos(angle);
    game->plane_x = game->plane_x * cos(angle) - game->plane_y * sin(angle);
    game->plane_y = old_plane_x * sin(angle) + game->plane_y * cos(angle);
}

void game_init(GameState *game)
{
    assets_init();
    assets_load_manifest("assets/textures.txt");
    ensure_defs_loaded();
    memset(game, 0, sizeof(*game));
    load_scenario(game, "assets/level.txt", "assets/logic.txt", 0);
}

void game_update(GameState *game, const PlatformInput *input, u32 delta_ms)
{
    const GameConfig *config;
    double move_speed;
    double mouse_turn_speed;
    double turn_speed;
    double strafe_x;
    double strafe_y;

    config = config_get();
    g_current_game = game;
    move_speed = ((double)delta_ms / 1000.0) * 2.8;
    turn_speed = ((double)delta_ms / 1000.0) * 1.8;
    mouse_turn_speed = ((double)config->mouse_sensitivity_milli) / 1000.0;
    strafe_x = game->plane_x / 0.66;
    strafe_y = game->plane_y / 0.66;

    if (game->weapon_flash_ms > 0) {
        game->weapon_flash_ms -= (int)delta_ms;
        if (game->weapon_flash_ms < 0) {
            game->weapon_flash_ms = 0;
        }
    }
    if (game->damage_flash_ms > 0) {
        game->damage_flash_ms -= (int)delta_ms;
        if (game->damage_flash_ms < 0) {
            game->damage_flash_ms = 0;
        }
    }

    if (input->forward) {
        double next_x;
        double next_y;

        next_x = game->player_x + game->dir_x * move_speed;
        next_y = game->player_y + game->dir_y * move_speed;
        if (can_move_player_to(next_x, game->player_y)) {
            game->player_x = next_x;
        }
        if (can_move_player_to(game->player_x, next_y)) {
            game->player_y = next_y;
        }
    }
    if (input->backward) {
        double next_x;
        double next_y;

        next_x = game->player_x - game->dir_x * move_speed;
        next_y = game->player_y - game->dir_y * move_speed;
        if (can_move_player_to(next_x, game->player_y)) {
            game->player_x = next_x;
        }
        if (can_move_player_to(game->player_x, next_y)) {
            game->player_y = next_y;
        }
    }
    if (input->strafe_left) {
        double next_x;
        double next_y;

        next_x = game->player_x - strafe_x * move_speed;
        next_y = game->player_y - strafe_y * move_speed;
        if (can_move_player_to(next_x, game->player_y)) {
            game->player_x = next_x;
        }
        if (can_move_player_to(game->player_x, next_y)) {
            game->player_y = next_y;
        }
    }
    if (input->strafe_right) {
        double next_x;
        double next_y;

        next_x = game->player_x + strafe_x * move_speed;
        next_y = game->player_y + strafe_y * move_speed;
        if (can_move_player_to(next_x, game->player_y)) {
            game->player_x = next_x;
        }
        if (can_move_player_to(game->player_x, next_y)) {
            game->player_y = next_y;
        }
    }

    if (input->turn_left) {
        rotate_player(game, -turn_speed);
    }
    if (input->turn_right) {
        rotate_player(game, turn_speed);
    }
    if (input->mouse_delta_x != 0 && mouse_turn_speed > 0.0) {
        rotate_player(game, (double)input->mouse_delta_x * mouse_turn_speed * ((double)delta_ms / 16.0));
    }

    if (input->use && !game->use_was_down) {
        if (!try_activate_use_trigger(game)) {
            try_open_door(game);
        }
    }
    if (input->next_weapon && !game->next_weapon_was_down) {
        select_next_weapon(game);
    }
    if (input->fire && !game->fire_was_down) {
        try_fire_weapon(game);
    }

    update_enter_triggers(game);
    update_doors(game, delta_ms);
    update_enemies(game, delta_ms);
    update_projectiles(game, delta_ms);
    update_pickups(game);

    if (game->pending_map_change && game->next_level_path[0] != '\0') {
        const char *logic_path;

        logic_path = game->next_logic_path[0] != '\0' ? game->next_logic_path : game->current_logic_path;
        load_scenario(game, game->next_level_path, logic_path, 1);
        return;
    }

    game->use_was_down = input->use;
    game->fire_was_down = input->fire;
    game->next_weapon_was_down = input->next_weapon;
}

void game_render(const GameState *game, u8 *framebuffer)
{
    g_current_game = (GameState *)game;
    render_world(game, framebuffer);
}

const PaletteColor *game_palette(void)
{
    return assets_get_palette();
}

int game_map_at(int x, int y)
{
    int tile;

    if (g_current_game == 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x >= g_current_game->map_width || y >= g_current_game->map_height) {
        return 1;
    }

    tile = raw_map_tile_at(g_current_game, x, y);
    if (tile == TILE_DOOR) {
        const DoorState *door;

        door = find_const_door(g_current_game, x, y);
        if (door != 0 && door->openness >= 0.95) {
            return TILE_EMPTY;
        }
    }

    return tile;
}

int game_player_health(const GameState *game)
{
    return game->health;
}

int game_player_ammo(const GameState *game)
{
    return current_weapon_ammo(game);
}

int game_player_ammo_type(const GameState *game)
{
    return current_weapon_ammo_type(game);
}

int game_player_keys(const GameState *game)
{
    return game->keys_mask;
}

int game_weapon_flash(const GameState *game)
{
    return game->weapon_flash_ms;
}

int game_damage_flash(const GameState *game)
{
    return game->damage_flash_ms;
}

int game_current_weapon(const GameState *game)
{
    return game->current_weapon_id;
}

int game_weapon_owned(const GameState *game, int weapon_id)
{
    return weapon_is_owned(game, weapon_id);
}

double game_door_openness_at(const GameState *game, int x, int y)
{
    const DoorState *door;

    door = find_const_door(game, x, y);
    if (door == 0) {
        return 0.0;
    }
    return door->openness;
}

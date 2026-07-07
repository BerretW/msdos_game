#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "platform.h"

typedef struct SpriteState {
    double x;
    double y;
    double vel_x;
    double vel_y;
    int texture_id;
    int active;
    int is_enemy;
    int is_projectile;
    int projectile_owner;
    int projectile_damage;
    int projectile_ttl_ms;
    int health;
    int cooldown_ms;
    int pickup_health;
    int pickup_ammo;
    int pickup_ammo_type_id;
    int pickup_key_mask;
    int pickup_weapon_id;
    int touch_damage;
    int speed_milli;
    int ai_mode;
    int alert_range_milli;
    int attack_range_milli;
    int attack_cooldown_ms;
} SpriteState;

typedef struct DoorState {
    int map_x;
    int map_y;
    double openness;
    int opening;
    int hold_open_ms;
    int required_key_mask;
} DoorState;

typedef struct TriggerState {
    int active;
    int id;
    int mode;
    int map_x;
    int map_y;
    int target_id;
    int once;
    int fired;
    int was_inside;
} TriggerState;

#define TRIGGER_MODE_USE 1
#define TRIGGER_MODE_ENTER 2

#define ACTUATOR_OPEN_DOOR 1
#define ACTUATOR_SPAWN 2
#define ACTUATOR_MAP 3
#define ACTUATOR_HEAL 4
#define ACTUATOR_GIVE_AMMO 5
#define ACTUATOR_GIVE_KEY 6
#define ACTUATOR_GIVE_WEAPON 7

typedef struct ActuatorState {
    int active;
    int target_id;
    int type;
    int map_x;
    int map_y;
    int value;
    int value1;
    char symbol;
    char text_arg0[64];
    char text_arg1[64];
} ActuatorState;

typedef struct GameState {
    double player_x;
    double player_y;
    double dir_x;
    double dir_y;
    double plane_x;
    double plane_y;
    int map[MAP_HEIGHT][MAP_WIDTH];
    SpriteState sprites[MAX_SPRITES];
    int sprite_count;
    DoorState doors[MAX_DOORS];
    int door_count;
    TriggerState triggers[MAX_TRIGGERS];
    int trigger_count;
    ActuatorState actuators[MAX_ACTUATORS];
    int actuator_count;
    int health;
    int ammo_by_type[MAX_AMMO_TYPES];
    int keys_mask;
    int current_weapon_id;
    int weapon_owned_mask;
    int fps_display;
    int weapon_flash_ms;
    int damage_flash_ms;
    int use_was_down;
    int fire_was_down;
    int next_weapon_was_down;
    char current_level_path[64];
    char current_logic_path[64];
    int pending_map_change;
    char next_level_path[64];
    char next_logic_path[64];
} GameState;

void game_init(GameState *game);
void game_update(GameState *game, const PlatformInput *input, u32 delta_ms);
void game_render(const GameState *game, u8 *framebuffer);
const PaletteColor *game_palette(void);
int game_map_at(int x, int y);
int game_player_health(const GameState *game);
int game_player_ammo(const GameState *game);
int game_player_ammo_type(const GameState *game);
int game_player_keys(const GameState *game);
int game_weapon_flash(const GameState *game);
int game_damage_flash(const GameState *game);
int game_current_weapon(const GameState *game);
int game_weapon_owned(const GameState *game, int weapon_id);
double game_door_openness_at(const GameState *game, int x, int y);

#endif

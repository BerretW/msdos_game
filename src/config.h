#ifndef CONFIG_H
#define CONFIG_H

typedef enum ConfigKey {
    CONFIG_KEY_NONE = 0,
    CONFIG_KEY_W,
    CONFIG_KEY_A,
    CONFIG_KEY_S,
    CONFIG_KEY_D,
    CONFIG_KEY_M,
    CONFIG_KEY_Q,
    CONFIG_KEY_E,
    CONFIG_KEY_F,
    CONFIG_KEY_SPACE,
    CONFIG_KEY_TAB,
    CONFIG_KEY_ESCAPE,
    CONFIG_KEY_UP,
    CONFIG_KEY_DOWN,
    CONFIG_KEY_LEFT,
    CONFIG_KEY_RIGHT
} ConfigKey;

typedef struct GameConfig {
    int fps_cap;
    int mouse_look;
    int mouse_fire;
    int mouse_sensitivity_milli;
    ConfigKey forward;
    ConfigKey backward;
    ConfigKey turn_left;
    ConfigKey turn_right;
    ConfigKey strafe_left;
    ConfigKey strafe_right;
    ConfigKey use;
    ConfigKey fire;
    ConfigKey next_weapon;
    ConfigKey toggle_minimap;
    ConfigKey quit;
} GameConfig;

void config_init(void);
const GameConfig *config_get(void);

#endif
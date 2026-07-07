#include "config.h"

#include <stdio.h>
#include <string.h>

static GameConfig g_config;
static int g_config_loaded;

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

static ConfigKey parse_key_name(const char *name)
{
    if (strcmp(name, "W") == 0) {
        return CONFIG_KEY_W;
    }
    if (strcmp(name, "A") == 0) {
        return CONFIG_KEY_A;
    }
    if (strcmp(name, "S") == 0) {
        return CONFIG_KEY_S;
    }
    if (strcmp(name, "D") == 0) {
        return CONFIG_KEY_D;
    }
    if (strcmp(name, "Q") == 0) {
        return CONFIG_KEY_Q;
    }
    if (strcmp(name, "E") == 0) {
        return CONFIG_KEY_E;
    }
    if (strcmp(name, "F") == 0) {
        return CONFIG_KEY_F;
    }
    if (strcmp(name, "SPACE") == 0) {
        return CONFIG_KEY_SPACE;
    }
    if (strcmp(name, "TAB") == 0) {
        return CONFIG_KEY_TAB;
    }
    if (strcmp(name, "ESC") == 0 || strcmp(name, "ESCAPE") == 0) {
        return CONFIG_KEY_ESCAPE;
    }
    if (strcmp(name, "UP") == 0) {
        return CONFIG_KEY_UP;
    }
    if (strcmp(name, "DOWN") == 0) {
        return CONFIG_KEY_DOWN;
    }
    if (strcmp(name, "LEFT") == 0) {
        return CONFIG_KEY_LEFT;
    }
    if (strcmp(name, "RIGHT") == 0) {
        return CONFIG_KEY_RIGHT;
    }
    return CONFIG_KEY_NONE;
}

static void set_binding(const char *action, ConfigKey key)
{
    if (key == CONFIG_KEY_NONE) {
        return;
    }

    if (strcmp(action, "forward") == 0) {
        g_config.forward = key;
    } else if (strcmp(action, "backward") == 0) {
        g_config.backward = key;
    } else if (strcmp(action, "turn_left") == 0) {
        g_config.turn_left = key;
    } else if (strcmp(action, "turn_right") == 0) {
        g_config.turn_right = key;
    } else if (strcmp(action, "strafe_left") == 0) {
        g_config.strafe_left = key;
    } else if (strcmp(action, "strafe_right") == 0) {
        g_config.strafe_right = key;
    } else if (strcmp(action, "use") == 0) {
        g_config.use = key;
    } else if (strcmp(action, "fire") == 0) {
        g_config.fire = key;
    } else if (strcmp(action, "next_weapon") == 0) {
        g_config.next_weapon = key;
    } else if (strcmp(action, "quit") == 0) {
        g_config.quit = key;
    }
}

static void load_defaults(void)
{
    memset(&g_config, 0, sizeof(g_config));
    g_config.fps_cap = 144;
    g_config.mouse_look = 1;
    g_config.mouse_fire = 1;
    g_config.mouse_sensitivity_milli = 3;
    g_config.forward = CONFIG_KEY_W;
    g_config.backward = CONFIG_KEY_S;
    g_config.turn_left = CONFIG_KEY_A;
    g_config.turn_right = CONFIG_KEY_D;
    g_config.strafe_left = CONFIG_KEY_Q;
    g_config.strafe_right = CONFIG_KEY_E;
    g_config.use = CONFIG_KEY_F;
    g_config.fire = CONFIG_KEY_SPACE;
    g_config.next_weapon = CONFIG_KEY_TAB;
    g_config.quit = CONFIG_KEY_ESCAPE;
}

void config_init(void)
{
    FILE *file;
    char resolved_path[260];
    char line[256];

    if (g_config_loaded) {
        return;
    }

    load_defaults();
    file = open_data_file("assets/config.txt", resolved_path);
    if (file != 0) {
        while (fgets(line, sizeof(line), file) != 0) {
            char command[32];
            char arg0[64];
            char arg1[64];

            if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
                continue;
            }

            command[0] = '\0';
            arg0[0] = '\0';
            arg1[0] = '\0';
            if (sscanf(line, " %31s %63s %63s", command, arg0, arg1) < 2) {
                continue;
            }

            if (strcmp(command, "fps_cap") == 0) {
                g_config.fps_cap = atoi(arg0);
                if (g_config.fps_cap < 0) {
                    g_config.fps_cap = 0;
                }
            } else if (strcmp(command, "mouse_look") == 0) {
                g_config.mouse_look = atoi(arg0) ? 1 : 0;
            } else if (strcmp(command, "mouse_fire") == 0) {
                g_config.mouse_fire = atoi(arg0) ? 1 : 0;
            } else if (strcmp(command, "mouse_sensitivity") == 0) {
                g_config.mouse_sensitivity_milli = atoi(arg0);
                if (g_config.mouse_sensitivity_milli < 0) {
                    g_config.mouse_sensitivity_milli = 0;
                }
            } else if (strcmp(command, "bind") == 0 && arg1[0] != '\0') {
                set_binding(arg0, parse_key_name(arg1));
            }
        }
        fclose(file);
    }

    g_config_loaded = 1;
}

const GameConfig *config_get(void)
{
    return &g_config;
}
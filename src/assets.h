#ifndef ASSETS_H
#define ASSETS_H

#include "common.h"

void assets_init(void);
void assets_load_manifest(const char *path);
void assets_try_load_raw(int texture_id, const char *path);
const u8 *assets_get_texture(int texture_id);
const PaletteColor *assets_get_palette(void);

#endif

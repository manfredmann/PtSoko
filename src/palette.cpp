/*
* PtSoko - Sokoban for QNX4.25/Photon
* Copyright (C) 2019-2020 Roman Serov <roman@serov.co>
*
* This file is part of Sokoban for QNX4.25/Photon.
* 
* Sokoban for QNX4.25/Photon is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Sokoban for QNX4.25/Photon is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Sokoban for QNX4.25/Photon. If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <string.h>

#include "palette.h"
#include "help.h"

Palette::Palette() {

}

Palette::~Palette() {
    for (size_t i = 0; i < colors.entries(); ++i) {
        delete colors[i];
    }

    colors.clear();
}

// Этот парсер убог как моя жизнь
void Palette::load(String fname) {
    FILE *f = fopen((const char *) fname, "r");

    if (f == NULL) {
        throw PaletteException("Couldn't open file '%s': %s", (const char *) fname, strerror(errno));
    }

    char buffer[256];

    while (fgets(buffer, 256, f) != NULL) {
        String line;

        for (size_t i = 0; i < 256; ++i) {
            if (buffer[i] == '\0') {
                break;
            }

            if (buffer[i] == '\n') {
                break;
            }

            if (buffer[i] == ' ') {
                continue;;
            }

            line += buffer[i];
        }

        char    sep = '=';
        int     sep_index = 0;
        int     sep_cnt = 0;

        while ((sep_index = line.index(sep, sep_index + 1)) != -1) {
            sep_cnt++;
        }

        if (sep_cnt == 0) {
            continue;
        }

        if (sep_cnt > 1) {
            fclose(f);

            throw PaletteException("Broken palette file");
        }

        sep_index = line.index(sep);

        if (sep_index == line.length() - 1) {
            continue;
        }

        String param_name;
        String param_value;

        param_name      = String(line, 0, sep_index);
        param_value     = String(line, sep_index + 1);

        palette_color_t *color = new palette_color_t;

        color->key = param_name;
        color->value = (unsigned int) strtoul((const char *) param_value, NULL, 16);

        colors.insert(color);
    }

    fclose(f);

    palette.background              = get_color("COLOR_BACKGROUND");
    palette.level_background        = get_color("COLOR_LEVEL_BACKGROUND");
    palette.status_background       = get_color("COLOR_STATUS_BACKGROUND");
    palette.status_font             = get_color("COLOR_STATUS_FONT");
    palette.font                    = get_color("COLOR_FONT");
    palette.menu_item_background    = get_color("COLOR_MENU_ITEM_BACKGROUND");
    palette.control_background      = get_color("COLOR_CONTROL_BACKGROUND");
    palette.control_btn             = get_color("COLOR_CONTROL_BTN");
    palette.control_btn_block       = get_color("COLOR_CONTROL_BTN_BLOCK");
    palette.control_btn_text        = get_color("COLOR_CONTROL_BTN_TEXT");
    palette.control_btn_text_block  = get_color("COLOR_CONTROL_BTN_TEXT_BLOCK");
    palette.control_btn_arm         = get_color("COLOR_CONTROL_BTN_ARM");
    palette.player                  = get_color("COLOR_PLAYER");
}
    
palette_t & Palette::get_palette() {
    return palette;
}

unsigned int Palette::get_color(String name) {
    for (size_t i = 0; i < colors.entries(); ++i) {
        palette_color_t *color = colors[i];

        if (color->key == name) {
            return color->value;
        }
    }

    throw PaletteException("Color name '%s' not found", (const char *) name);
}

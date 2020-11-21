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

#include "help.h"
#include <dirent.h>

String Help::Sprintf(const char *fmt, ...) {
    char *  buffer = new char[1024];
    String  str;

    va_list args;
    va_start(args, fmt);
    _vbprintf(buffer, 1024, fmt, args);
    va_end(args);

    str = String(buffer);
    delete[] buffer;

    return str;
}

bool Help::is_dir_exists(String path) {
    DIR *dir = opendir((const char *) path);

    if (dir != NULL) {
        closedir(dir);

        return true;
    }

    return false;
}

bool Help::is_file_exists(String path) {
    FILE *f = fopen((const char *) path, "r");

    if (f != NULL) {
        fclose(f);

        return true;
    }

    return false;
}

unsigned int Help::get_string_width(const char *font, char *str) {
    PhRect_t rect;

    PfExtentText(&rect, NULL, font, str, strlen(str));

    return rect.lr.x - rect.ul.x + 1;
}

unsigned int Help::get_string_height(const char *font, char *str) {
    PhRect_t rect;

    PfExtentText(&rect, NULL, font, str, strlen(str));

    return rect.lr.y - rect.ul.y + 1;
}

void Help::draw_string(unsigned int x, unsigned int y, char *str, char *font, unsigned int color, unsigned int background_color, bool back) {
    PhPoint_t p;
    p.x = x;
    p.y = y;

    unsigned short h = Help::get_string_height(font, str);
    unsigned short w = Help::get_string_width(font, str);

    if (back) {
        PgSetFillColor(background_color);
        PgDrawIRect(p.x, p.y - h, p.x + w, p.y, Pg_DRAW_FILL);
    }

    PgSetFont(font);
    PgSetTextColor(color);
    PgDrawText(str, strlen(str), &p, 0);
}

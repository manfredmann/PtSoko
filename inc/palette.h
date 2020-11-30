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

#ifndef PALETTE_H
#define PALETTE_H

#include <String.h>
#include <stdio.h>
#include <wcvector.h>
#include <stdarg.h>

#include "exception.h"

typedef struct {
    String          key;
    unsigned int    value;
} palette_color_t;

typedef struct {
    unsigned int background;
    unsigned int level_background;
    unsigned int status_background;
    unsigned int status_font;
    unsigned int font;
    unsigned int menu_item_background;
    unsigned int control_background;
    unsigned int player;

    unsigned int control_btn;
    unsigned int control_btn_block;

    unsigned int control_btn_text;
    unsigned int control_btn_text_block;

    unsigned int control_btn_arm;
} palette_t;

// Для этой срани нужно писать свою хеш-функцию, а мне лень
// typedef WCValHashDict<palette_key_t, unsigned int> palette_map_t;

typedef WCValOrderedVector<palette_color_t *> palette_colors_t;

//Вещества были забористые
class PaletteException : public BaseException {
    public:
        PaletteException(String msg) : BaseException(msg) { } 
        PaletteException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

class Palette {
    public:
        Palette();
        ~Palette();

        void                load(String fname);
        palette_t &         get_palette();

    private:

        unsigned int        get_color(String name);

        palette_colors_t    colors;
        palette_t           palette;
};

#endif

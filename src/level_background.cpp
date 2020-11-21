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

#include "level_background.h"

Level_background::Level_background(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PgColor_t color) {
    this->pos.x = x;
    this->pos.y = y;
    this->pos.w = w;
    this->pos.h = h;
    this->color = color;

    this->is_changed = true;
}

void Level_background::draw() {
    if (is_changed) {
        PhRect_t  rect;

        rect.ul.x = pos.x;
        rect.ul.y = pos.y;
        rect.lr.x = pos.x + pos.w;
        rect.lr.y = pos.y + pos.h;

        PgSetUserClip(&rect);

        PgSetFillColor(color);
        PgDrawIRect(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, Pg_DRAW_FILL);      

        PtClipRemove();

        is_changed = false;
    }
}

object_type_t Level_background::get_type() {
    return OBJECT_BACKGROUND;
}

void Level_background::set_pos(unsigned int x, unsigned int y) {
    this->pos.x = x;
    this->pos.y = y;
}

object_pos_t Level_background::get_pos() {
    return pos;
}

void Level_background::set_changed() {
    is_changed = true;
}

bool Level_background::get_changed() {
    return is_changed;
}

/*
* PtSoko - Sokoban for QNX4.25/Photon
* Copyright (C) 2019 Roman Serov <roman@serov.co>
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

#include "box.h"

Box::Box(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture) {
    this->pos.x = x;
    this->pos.y = y;
    this->pos.w = w;
    this->pos.h = h;

    this->texture = texture;
    this->is_changed = true;
}

void Box::draw() {
    if (is_changed) {
        PhRect_t  rect;

        rect.ul.x = pos.x;
        rect.ul.y = pos.y;
        rect.lr.x = pos.x + pos.w;
        rect.lr.y = pos.y + pos.h;

        PgSetUserClip(&rect);

        if (texture == NULL) {
            PgSetFillColor(0xAA5500);
            PgDrawIRect(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, Pg_DRAW_FILL);
        } else {
            PhPoint_t    p = { pos.x, pos.y };

            PgDrawImagemx(texture->image, texture->type, &p, &texture->size, texture->bpl, 0 );
        }

        is_changed = false;
    }

    PtClipRemove();
}

object_type_t Box::get_type() {
    return OBJECT_BOX;
}

void Box::set_pos(unsigned int x, unsigned int y) {
    this->pos.x = x;
    this->pos.y = y;

    is_changed = true;
}

object_pos_t Box::get_pos() {
    return pos;
}

object_pos_t Box::move_calc(direction_t dir) {
    object_pos_t pos_next = pos;

    switch(dir) {
        case DIRECTION_UP: {
            pos_next.y = pos_next.y - pos.h - 1;
            break;
        }
        case DIRECTION_DOWN: {
            pos_next.y = pos_next.y + pos.h + 1;
            break;
        }
        case DIRECTION_LEFT: {
            pos_next.x = pos_next.x - pos.w - 1;
            break;
        }
        case DIRECTION_RIGHT: {
            pos_next.x = pos_next.x + pos.w + 1;
            break;
        }
    }

    return pos_next;
}

void Box::move(direction_t dir) {
    pos = move_calc(dir);

    is_changed = true;
}

void Box::set_changed() {
    is_changed = true;
}

bool Box::get_changed() {
    return is_changed;
}

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

#include <stdio.h>

#include "level_preview.h"

Level_preview::Level_preview(PhPoint_t pos, PhDim_t size, unsigned int block_size, textures_t &textures, palette_t &palette) : textures(textures), palette(palette) {
    this->pos           = pos;
    this->size          = size;
    this->block_size    = block_size;
    this->lvl           = NULL;
}

void Level_preview::load_level(Level *lvl) {
    this->lvl = lvl;

    while (objects.entries() != 0) {
        Object *obj = objects.first();

        switch(obj->get_type()) {
            case OBJECT_BRICK: {
                delete (Brick *) obj;
                break;
            }
            case OBJECT_BOX: {
                delete (Box *) obj;
                break;
            }
            case OBJECT_BOXPLACE: {
                delete (Box_place *) obj;
                break;
            }
            case OBJECT_PLAYER: {
                delete (Player *) obj;
                break;
            }
            case OBJECT_BACKGROUND: {
                delete (Level_background *) obj;
                break;              
            }
        }

        objects.removeFirst();
    }

    while (background.entries() != 0) {
        background.removeFirst();
    }

    while (boxes.entries() != 0) {
        boxes.removeFirst();
    }

    while (box_places.entries() != 0) {
        box_places.removeFirst();
    }

// ===============================

    String level = lvl->get_level_str();

    size_t          i;

    unsigned int    x           = 0;
    unsigned int    y           = 0;
    
    unsigned int    x_max       = 0;
    unsigned int    y_max       = 0;
    
    unsigned int    x_offset    = 0;
    unsigned int    y_offset    = 0;

    unsigned int    step        = block_size;

    Player *        player      = NULL;

    for (i = 0; i < level.length(); ++i) {
        char type = level[i];

        switch(type) {
            case LVL_BTYPE_BRICK: {
                Brick *brick = new Brick(x, y, step - 1, step - 1, textures.brick);

                objects.insert((Object *)brick);
                break;
            }
            case LVL_BTYPE_BOX: {
                Level_background *  lvl_back = new Level_background(x, y, step - 1, step - 1, palette.level_background);
                Box *               box = new Box(x, y, step - 1, step - 1, textures.box);

                boxes.insert((Object *)box);
                background.insert((Object *) lvl_back);

                break;
            }
            case LVL_BTYPE_BOX_PLACE: {
                Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures.box_place);

                box_places.insert((Object *)box_place);
                break;
            }
            case LVL_BTYPE_BOXWPLACE: {
                Box *box = new Box(x, y, step - 1, step - 1, textures.box);
                Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures.box_place);

                boxes.insert((Object *)box);
                box_places.insert((Object *)box_place);
                break;
            }
            case LVL_BTYPE_PLAYER: {
                player = new Player(x, y, step - 1, step - 1);

                Level_background *lvl_back = new Level_background(x, y, step - 1, step - 1, palette.level_background);
                background.insert((Object *) lvl_back);

                break;
            }
            case LVL_BTYPE_BACKGROUND: {
                Level_background *lvl_back = new Level_background(x, y, step - 1, step - 1, palette.level_background);

                background.insert((Object *) lvl_back);
                break;
            }
            case '\n': {
                y += step;

                if (y_max < y) {
                    y_max = y;
                }

                x = 0;
                continue;
            }
        }

        x += step;

        if (x_max < x) {
            x_max = x;
        }
    }

    x_offset = pos.x + (size.w - x_max) / 2;
    y_offset = pos.y + (size.h - y_max) / 2;

    for (i = 0; i < background.entries(); ++i) {
        objects.insert(background[i]);
    }

    for (i = 0; i < box_places.entries(); ++i) {
        objects.insert(box_places[i]);
    }

    for (i = 0; i < boxes.entries(); ++i) {
        objects.insert(boxes[i]);
    }

    objects.insert((Object *)player);

    for (i = 0; i < objects.entries(); ++i) {
        Object *obj = objects[i];

        object_pos_t obj_pos = obj->get_pos();
        obj_pos.x += x_offset;
        obj_pos.y += y_offset;
        obj->set_pos(obj_pos.x, obj_pos.y);
    }
}

void Level_preview::draw() {
    PhRect_t  rect;
    
    rect.ul.x = pos.x;
    rect.ul.y = pos.y;
    rect.lr.x = pos.x + size.w;
    rect.lr.y = pos.y + size.h;

    PgSetUserClip(&rect);

    PgSetFillColor(palette.background);
    PgDrawRect(&rect, Pg_DRAW_FILL);

    char *info_font = "pcterm14";

    if (lvl != NULL) {
        char str[100];

        String level_name = lvl->get_name();

        _bprintf(str, 100, "Level preview: %s", (const char *) level_name);

        unsigned int w = Help::get_string_width(info_font, str);
        unsigned int h = Help::get_string_height(info_font, str);
        unsigned int x = pos.x;
        unsigned int y = pos.y + h * 2;

        x += (size.w - w) / 2;

        Help::draw_string(x, y, str, info_font, palette.font);

        for (size_t i = 0; i < objects.entries(); ++i) {
            Object *object = objects[i];
            object->set_changed();
            object->draw();
        }

        if (!lvl->is_first_run()) {
            x = pos.x;
            y = pos.y;

            const level_stat_t &best_run = lvl->get_stat_best();

            _bprintf(str, 100, "Best run: %02d:%02d / %d", 
                best_run.time.minutes, 
                best_run.time.seconds,
                best_run.moves);

            x += (size.w - Help::get_string_width(info_font, str)) / 2;
            y += size.h - (h * 2);

            Help::draw_string(x, y, str, info_font, palette.font);
        }
    }

    PtClipRemove();
}

void Level_preview::set_pos(PhPoint_t pos) {
    this->pos = pos;
}

void Level_preview::set_size(PhDim_t size) {
    this->size = size;
}

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

#ifndef MENU_H
#define MENU_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>
#include <bool.h>
#include <wcvector.h>

#include "palette.h"
#include "level.h"
#include "help.h"

class MenuItem {
    public:
        MenuItem(String name, void *data) {
            this->name      = name;
            this->selected  = false;
            this->data      = data;
        }

        bool is_selected() {
            return selected;
        }

        void set_selected(bool sel) {
            this->selected = sel;
        }

        String get_name() {
            return name;
        }

        void *get_data() {
            return data;
        }

    private:
        String  name;
        bool    selected;
        void *  data;
};

typedef void(*menu_item_enter_clbk_t)(String name, void *data);
typedef void(*menu_item_select_clbk_t)(String name, void *data);

typedef WCValOrderedVector<MenuItem *> menu_items_t;

class Menu {
    public:
        Menu(PhDim_t size, PhPoint_t pos, palette_t &palette);

        void                    add_item(String name, void *data = NULL);
        void                    add_enter_callback(menu_item_enter_clbk_t callback);
        void                    add_select_callback(menu_item_select_clbk_t callback);
        void                    draw();

        void                    set_pos(PhPoint_t pos);
        void                    set_size(PhDim_t size);

        void                    up();
        void                    down();
        void                    left();
        void                    right();
        void                    enter();

        void                    select(size_t index);

        PhDim_t                 get_size();
        PhPoint_t               get_pos();

    private:
        palette_t &             palette;
        PhDim_t                 size;
        PhPoint_t               pos;
        unsigned int            visible_items;
        unsigned int            current_item;
        unsigned int            current_visible_item;
        menu_item_enter_clbk_t  enter_callback;
        menu_item_select_clbk_t select_callback;

        menu_items_t            items;
};

#endif

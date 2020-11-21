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

#include "menu.h"

Menu::Menu(PhDim_t size, PhPoint_t pos, palette_t &palette) : palette(palette) {
    this->size              = size;
    this->pos               = pos;

    this->enter_callback    = NULL;
    this->select_callback   = NULL;
}

void Menu::add_item(String name, void *data) {
    MenuItem *item = new MenuItem(name, data);

    items.insert(item);

    items[0]->set_selected(true);
    current_item = 0;
}

void Menu::draw() {
    char * item_font = "pcterm14";

    unsigned int font_height    = Help::get_string_height(item_font, "a");
    unsigned int item_height    = font_height + 10;
    unsigned int item_width     = size.w;
    unsigned int separator      = 10;

    PhRect_t  rect;

    rect.ul.x = pos.x;
    rect.ul.y = pos.y;
    rect.lr.x = pos.x + size.w;
    rect.lr.y = pos.y + size.h;

    PgSetUserClip(&rect);

    PgSetFillColor(palette.background);
    PgDrawRect(&rect, Pg_DRAW_FILL);

    int x = pos.x;
    int y = pos.y;

    visible_items = size.h / (item_height + separator);

    y += ((size.h - (visible_items * (item_height + separator))) / 2);

    current_visible_item = ((current_item / visible_items) * visible_items);

    for (size_t i = current_visible_item; i < current_visible_item + visible_items; ++i) {
        if (i >= items.entries()) {
            break;
        }

        y += (separator / 2);

        MenuItem *  item = items[i];
        PhRect_t    item_rect;

        rect.ul.x = x;
        rect.ul.y = y;
        rect.lr.x = x + size.w;
        rect.lr.y = y + item_height - 1;

        if (item->is_selected()) {
            PgSetFillColor(palette.menu_item_background);
            PgDrawIRect(rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, Pg_DRAW_FILL);            
        }

        char str[100];
        _bprintf(str, 100, "%s", (const char *) item->get_name());

        if (Help::get_string_width(item_font, str) >= item_width) {
            int temp_w      = 0;
            int len         = strlen(str);
            char *temp_str  = new char[len + 1];
            strcpy(temp_str, str);

            do {
                temp_str[len--] = '\0';

                temp_w = Help::get_string_width(item_font, temp_str);
            } while(temp_w >= item_width);

            temp_str[len]     = '.';
            temp_str[len - 1] = '.';
            temp_str[len - 2] = '.';

            strcpy(str, temp_str);
        }

        Help::draw_string(x + (item_width / 2) - (Help::get_string_width(item_font, str) / 2), 
            (y + (item_height / 2) + (font_height / 2) - 2), 
            str, 
            item_font, 
            palette.font);

        y += item_height + (separator / 2);
    }

    PtClipRemove();
}

void Menu::up() {
    if (current_item > 0) {
        items[current_item]->set_selected(false);
        --current_item;
        items[current_item]->set_selected(true);

        if (select_callback) {
            MenuItem *item = items[current_item];

            select_callback(item->get_name(), item->get_data());
        }
    }
}

void Menu::down() {
    if (current_item + 1 < items.entries()) {
        items[current_item]->set_selected(false);
        ++current_item;
        items[current_item]->set_selected(true);

        if (select_callback) {
            MenuItem *item = items[current_item];

            select_callback(item->get_name(), item->get_data());
        }
    }
}

void Menu::left() {
    int nvi = (int) current_visible_item - 1;

    if (nvi < 0) {
        nvi = 0;
    }

    items[current_item]->set_selected(false);
    current_item = nvi;
    items[current_item]->set_selected(true);

    if (select_callback) {
        MenuItem *item = items[current_item];

        select_callback(item->get_name(), item->get_data());                
    }
}

void Menu::right() {
    int nvi = current_visible_item + visible_items;

    if (nvi >= items.entries()) {
        nvi = items.entries() - 1;
    }

    items[current_item]->set_selected(false);
    current_item = nvi;
    items[current_item]->set_selected(true);

    if (select_callback) {
        MenuItem *item = items[current_item];

        select_callback(item->get_name(), item->get_data());                
    }
}

void Menu::enter() {
    if (enter_callback) {
        MenuItem *item = items[current_item];

        enter_callback(item->get_name(), item->get_data());                
    }
}

void Menu::select(size_t index) {
    items[current_item]->set_selected(false);
    current_item = index;
    items[current_item]->set_selected(true);

    if (select_callback) {
        MenuItem *item = items[current_item];

        select_callback(item->get_name(), item->get_data());                
    }
}

void Menu::add_enter_callback(menu_item_enter_clbk_t callback) {
    enter_callback = callback;
}

void Menu::add_select_callback(menu_item_select_clbk_t callback) {
    select_callback = callback;
}

void Menu::set_pos(PhPoint_t pos) {
    this->pos = pos;
}

void Menu::set_size(PhDim_t size) {
    this->size = size;
}

PhDim_t Menu::get_size() {
    return this->size;
}

PhPoint_t Menu::get_pos() {
    return this->pos;
}

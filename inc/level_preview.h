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

#ifndef LEVEL_PREVIEW_H
#define LEVEL_PREVIEW_H

#include "palette.h"
#include "textures.h"
#include "level.h"
#include "object.h"
#include "brick.h"
#include "box.h"
#include "box_place.h"
#include "level_background.h"
#include "player.h"

class Level_preview {
    public:
        Level_preview(PhPoint_t pos, PhDim_t size, unsigned int block_size, textures_t &textures, palette_t &palette);

        void            set_pos(PhPoint_t pos);
        void            set_size(PhDim_t size);

        void            load_level(Level *lvl);
        void            draw();

    private:
        PhPoint_t       pos;
        PhDim_t         size;

        textures_t &    textures;
        palette_t &     palette;

        unsigned int    block_size;

        objects_t       objects;
        objects_t       boxes;
        objects_t       box_places;
        objects_t       background;

        Level *         lvl;
};

#endif

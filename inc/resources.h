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

#ifndef RES_H
#define RES_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>

#include <wcvector.h>
#include <String.h>
#include <debug.h>
#include <stdarg.h>

#include "object.h"
#include "level.h"
#include "textures.h"
#include "palette.h"
#include "exception.h"

typedef WCValOrderedVector<String>          homes_t;
typedef WCValOrderedVector<String>          paths_t;
typedef WCValOrderedVector<void *>          shmem_ptrs_t;
typedef WCValSortedVector<unsigned short>   texture_sizes_t;

//Вещества были забористые
class ResourcesException : public BaseException {
    public:
        ResourcesException(String msg) : BaseException(msg) { } 
        ResourcesException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

class Resources {
    public:
        static Resources &get_instance() {
            static Resources res;

            return res;
        }

        void                init();

        void                load_textures(PhDim_t win_size, PhDim_t preview_size);
        void                load_levels();
        void                load_palette();

        String              get_path_home();
        String              get_user_home();
        String              get_path_textures();
        String              get_path_levels();
        String              get_path_stat();

        unsigned short      get_block_size();
        unsigned short      get_preview_block_size();

        textures_t *        get_textures();
        textures_t *        get_preview_textures();

        levels_t *          get_levels();
        Level *             get_level(size_t index);

        palette_t           get_palette();

        ~Resources();

    private:
        Resources();

        void                find_home();
        static void *       img_memory_allocate(long nbytes, int type);
        static void *       img_memory_free(void *memory, int type);
        static void *       img_warning(char *msg);
        static void *       img_error(char *msg);
        static void *       img_progress(int percent);

        String              texture_find(String path);
        
        Debug &             debug;

        paths_t             paths;
        homes_t             homes;

        Palette             palette;

        String              path_home;
        String              path_textures;
        String              path_levels;
        String              path_stat;

        texture_sizes_t     texture_sizes;
        textures_t          textures;
        textures_t          preview_textures;

        levels_t            levels;

        shmem_ptrs_t        img_shmem;

        unsigned short      block_size;
        unsigned short      preview_block_size;

};

#endif

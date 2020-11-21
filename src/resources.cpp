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

#include "resources.h"
#include "help.h"

#include <dirent.h>

Resources::Resources() : debug(Debug::get_instance()) {
    homes.insert("./.ptsoko/");
    homes.insert(String(String(getenv("HOME")) + "/.ptsoko/"));
    homes.insert("/usr/ptsoko/");
}

Resources::~Resources() {
    for (size_t i = 0; i < img_shmem.entries(); ++i) {
        PgShmemDestroy(img_shmem[i]);
    }
}

void Resources::init() {
    debug.printf(DBG_LVL_3, "call Resources::init\n");

    find_home();
}

void *Resources::img_memory_allocate(long nbytes, int type) {
    Resources &  res = Resources::get_instance();

    res.debug.printf(DBG_LVL_3, "call Resources::img_memory_allocate\n");

    if (type == PX_IMAGE) {
        void *  img_shmem_ptr   = PgShmemCreate(nbytes, NULL);

        res.img_shmem.insert(img_shmem_ptr);

        return img_shmem_ptr;
    } else {
        return calloc(1, nbytes);
    }
}

void *Resources::img_memory_free(void *memory, int type) {   
    Resources &  res = Resources::get_instance();

    res.debug.printf(DBG_LVL_3, "call Resources::img_memory_free\n");

    if (type == PX_IMAGE) {
        for (size_t i = 0; i < res.img_shmem.entries(); ++i) {
            if (res.img_shmem[i] == memory) {
                res.img_shmem.removeAt(i);

                break;
            }
        }

        PgShmemDestroy(memory);
    } else {
        free(memory);
    }

    return NULL;
}

void *Resources::img_warning(char *msg) {
    Resources &  res = Resources::get_instance();

    res.debug.printf(DBG_LVL_3, "call Resources::img_warning\n");
    res.debug.printf(DBG_LVL_1, "%s\n", msg);

    return NULL;
}

void *Resources::img_error(char *msg) {
    Resources &  res = Resources::get_instance();

    res.debug.printf(DBG_LVL_3, "call Resources::img_error\n");
    res.debug.printf(DBG_LVL_1, "%s\n", msg);

    exit(EXIT_FAILURE);

    return NULL;
}

void *Resources::img_progress(int percent) {
    Resources &  res = Resources::get_instance();

    res.debug.printf(DBG_LVL_3, "call Resources::img_progress\n");
    //printf("Texture load status:  %d.%d percent\n", percent >> 16, percent & 0xffff);
    return NULL;
}

void Resources::load_textures(PhDim_t win_size, PhDim_t preview_size) {
    debug.printf(DBG_LVL_3, "call Resources::load_textures\n");

    PxMethods_t methods_px;
    
    memset(&methods_px, 0, sizeof(PxMethods_t));

    methods_px.px_alloc         = Resources::img_memory_allocate;
    methods_px.px_free          = Resources::img_memory_free;
    methods_px.px_warning       = Resources::img_warning;
    methods_px.px_error         = Resources::img_error;
    methods_px.px_progress      = Resources::img_progress;
    methods_px.flags            = PX_LOAD;

// ============================

    String textures_path[3];

    texture_sizes.insert(18);
    texture_sizes.insert(24);
    texture_sizes.insert(30);
    texture_sizes.insert(36);
    texture_sizes.insert(42);

    size_t i;

    for (i = 0; i < texture_sizes.entries(); ++i) {
        unsigned short b_size = texture_sizes[i];       

        unsigned short w = LVL_MAX_BLOCKS_W * b_size;
        unsigned short h = LVL_MAX_BLOCKS_H * b_size;

        if (w > win_size.w || h > win_size.h) {
            break;
        }

        block_size = b_size;
    }

    debug.printf(DBG_LVL_1, "Level block size: %d\n", block_size);

    for (i = 0; i < texture_sizes.entries(); ++i) {
        unsigned short b_size = texture_sizes[i];       

        unsigned short w = LVL_MAX_BLOCKS_W * b_size;
        unsigned short h = LVL_MAX_BLOCKS_H * b_size;

        if (w > preview_size.w || h > preview_size.h) {
            break;
        }

        preview_block_size = b_size;
    }

    debug.printf(DBG_LVL_1, "Level preview block size: %d\n", preview_block_size);

// ============================

    textures_path[0] = texture_find(Help::Sprintf("box_%d.bmp", block_size));
    textures_path[1] = texture_find(Help::Sprintf("box_place_%d.bmp", block_size));
    textures_path[2] = texture_find(Help::Sprintf("brick_%d.bmp", block_size));

    if (textures_path[0].length() == 0 || textures_path[1].length() == 0 || textures_path[2].length() == 0) {
        throw ResourcesException("Textures not found");
    }

    textures.box        = PxLoadImage(strdup((const char *)textures_path[0]), &methods_px);
    textures.box_place  = PxLoadImage(strdup((const char *)textures_path[1]), &methods_px);
    textures.brick      = PxLoadImage(strdup((const char *)textures_path[2]), &methods_px);

    if (textures.box->size.w != block_size) {
        throw ResourcesException("Texture width must be: %d", block_size);
    }

    if (textures.box_place->size.w != block_size) {
        throw ResourcesException("Texture width must be: %d", block_size);
    }

    if (textures.brick->size.w != block_size) {
        throw ResourcesException("Texture width must be: %d", block_size);
    }

    if (textures.box->size.h != block_size) {
        throw ResourcesException("Texture height must be: %d", block_size);
    }

    if (textures.box_place->size.h != block_size) {
        throw ResourcesException("Texture height must be %d", block_size);
    }

    if (textures.brick->size.h != block_size) {
        throw ResourcesException("Texture height must be %d", block_size);
    }

    textures_path[0] = texture_find(Help::Sprintf("box_%d.bmp",         preview_block_size));
    textures_path[1] = texture_find(Help::Sprintf("box_place_%d.bmp",   preview_block_size));
    textures_path[2] = texture_find(Help::Sprintf("brick_%d.bmp",       preview_block_size));

    if (textures_path[0].length() == 0 || textures_path[1].length() == 0 || textures_path[2].length() == 0) {
        throw ResourcesException("preview textures not found");
    }

// ============================

    preview_textures.box        = PxLoadImage(strdup((const char *)textures_path[0]), &methods_px);
    preview_textures.box_place  = PxLoadImage(strdup((const char *)textures_path[1]), &methods_px);
    preview_textures.brick      = PxLoadImage(strdup((const char *)textures_path[2]), &methods_px);

    if (preview_textures.box->size.w != preview_block_size) {
        throw ResourcesException("Preview texture width must be: %d", preview_block_size);
    }

    if (preview_textures.box_place->size.w != preview_block_size) {
        throw ResourcesException("Preview texture width must be: %d", preview_block_size);
    }

    if (preview_textures.brick->size.w != preview_block_size) {
        throw ResourcesException("Preview texture width must be: %d", preview_block_size);
    }

    if (preview_textures.box->size.h != preview_block_size) {
        throw ResourcesException("Preview texture height must be: %d", preview_block_size);
    }

    if (preview_textures.box_place->size.h != preview_block_size) {
        throw ResourcesException("Preview texture height must be %d", preview_block_size);
    }

    if (preview_textures.brick->size.h != preview_block_size) {
        throw ResourcesException("Preview texture height must be %d", preview_block_size);
    }
}

String Resources::texture_find(String path) {
    debug.printf(DBG_LVL_3, "call Resources::texture_find\n");

    String texture_path = String(path_textures + path);

    FILE *f = fopen((const char *) texture_path, "r");

    if (f == NULL) {
        throw ResourcesException("error: Couldn't open file \"%s\"\n", (const char *)texture_path);
    }
    
    debug.printf(DBG_LVL_1, "Texture: %s OK\n",  (const char *)texture_path);

    fclose(f);

    return texture_path;
}

void Resources::load_levels() {
    debug.printf(DBG_LVL_3, "call Resources::load_levels\n");

    DIR *           dir = NULL;
    struct dirent * entry;
    size_t          i;

    debug.printf(DBG_LVL_1, "Searching levels at \"%s\"\n", (const char *) path_levels);

    dir = opendir((const char *) path_levels);

    if (!dir) {
        throw ResourcesException("Levels not found\n");
    }

    WCValSortedVector<String> files;

    while ((entry = readdir(dir)) != NULL) {
        String fname = String(entry->d_name);

        int i = fname.index(".lvl", fname.length() - 4);

        if (i == -1) {
            continue;
        }

        files.insert(fname);
    }

    for (i = 0; i < files.entries(); ++i) {
        String fname = files[i];

        debug.printf(DBG_LVL_1, "Loading %s: ", (const char *) fname);

        Level *lvl = new Level();

        try {
            lvl->load(path_levels + fname, fname);
        } catch (LevelException &e) {
            debug.printf(DBG_LVL_1, "Level loading error: %s\n", (const char *) e.what());
            delete lvl;

            continue;
        }

        if (lvl->get_width() > LVL_MAX_BLOCKS_W) {
            debug.printf(DBG_LVL_1, "Broken level file. Level width to high. Must be <= %d\n", LVL_MAX_BLOCKS_W);

            continue;
        }

        if (lvl->get_height() > LVL_MAX_BLOCKS_H) {
            debug.printf(DBG_LVL_1, "Broken level file. Level height to high. Must be <= %d\n", LVL_MAX_BLOCKS_H);

            continue;
        }

        if (lvl->get_player_count() == 0) {
            debug.printf(DBG_LVL_1, "Broken level file. Player = 0\n");

            continue;
        }

        if (lvl->get_boxes_count() == 0) {
            debug.printf(DBG_LVL_1, "Broken level file. Box = 0\n");

            continue;
        }

        if (lvl->get_box_places_count() == 0) {
            debug.printf(DBG_LVL_1, "Broken level file. Box_place = 0\n");

            continue;
        }

        if (lvl->get_boxes_count() != lvl->get_box_places_count()) {
            debug.printf(DBG_LVL_1, "Broken level file. Box != Box_place\n");

            continue;
        }

        if (lvl->get_player_count() > 1) {
            debug.printf(DBG_LVL_1, "Broken level file. Player > 1\n");

            continue;
        }
    
        levels.insert(lvl);

        debug.printf(DBG_LVL_1, "OK\n");

        try {
            String stat_fname = Help::Sprintf("%s/%s.dat", (const char *) path_stat, (const char *) lvl->get_name());

            lvl->load_stat(stat_fname);
        } catch (StatisticException &e) {
            debug.printf(DBG_LVL_1, "Couldn't load statistic file (first run?): %s\n", e.what_c());
        }
    }

    closedir(dir);

    if (levels.entries() == 0) {
        throw ResourcesException("Levels not found");
    }     
}

void Resources::load_palette() {
    debug.printf(DBG_LVL_3, "call Resources::load_palette\n");

    String palette_path = path_home + String("palette.dat");

    debug.printf(DBG_LVL_1, "Loading palete: %s ", (const char *) palette_path);

    try {
        palette.load(palette_path);
        debug.printf(DBG_LVL_1, "OK\n");
    } catch (PaletteException &e) {
        throw ResourcesException(e.what());
    }
}

void Resources::find_home() {
    debug.printf(DBG_LVL_3, "call Resources::find_home\n");

    debug.printf(DBG_LVL_1, "Searching home...\n");

    bool is_find = false;

    for (size_t i = 0; i < homes.entries(); ++i) {
        path_home = homes[i];

        debug.printf(DBG_LVL_1, "===================================================\n");
        debug.printf(DBG_LVL_1, "Home:      %s: ", (const char *) path_home);
        
        if (Help::is_dir_exists(path_home)) {
            debug.printf(DBG_LVL_1, "OK\n");

            path_levels     = Help::Sprintf("%slevels/",     (const char *) path_home);
            path_textures   = Help::Sprintf("%stextures/",   (const char *) path_home);
            path_stat       = Help::Sprintf("%sstat/",       (const char *) path_home);

            debug.printf(DBG_LVL_1, "Levels:    %s: ", (const char *) path_levels);

            if (!Help::is_dir_exists(path_levels)) {
                debug.printf(DBG_LVL_1, "Not found\n");
                continue;
            }

            debug.printf(DBG_LVL_1, "OK\n");
            debug.printf(DBG_LVL_1, "Textures:  %s: ", (const char *) path_textures);            

            if (!Help::is_dir_exists(path_textures)) {
                debug.printf(DBG_LVL_1, "Not found\n");
                continue;
            }

            debug.printf(DBG_LVL_1, "OK\n");
            debug.printf(DBG_LVL_1, "Statistic: %s: ", (const char *) path_stat);            

            if (!Help::is_dir_exists(path_stat)) {
                debug.printf(DBG_LVL_1, "Not found\n");
                continue;
            }

            debug.printf(DBG_LVL_1, "OK\n");

            is_find = true;

            break;
        } else {
            debug.printf(DBG_LVL_1, "Not found\n");
        }
    }

    if (!is_find) {
        throw ResourcesException("Home directory structure not found");
    }
}

String Resources::get_path_home() {
    debug.printf(DBG_LVL_3, "call Resources::get_path_home\n");

    return path_home;
}

String Resources::get_user_home() {
    debug.printf(DBG_LVL_3, "call Resources::get_user_home\n");

    return String(getenv("HOME"));
}

String Resources::get_path_textures() {
    debug.printf(DBG_LVL_3, "call Resources::get_path_textures\n");

    return path_textures;
}

String Resources::get_path_levels() {
    debug.printf(DBG_LVL_3, "call Resources::get_path_levels\n");

    return path_levels;
}

String Resources::get_path_stat() {
    debug.printf(DBG_LVL_3, "call Resources::get_path_stat\n");

    return path_stat;
}

unsigned short Resources::get_block_size() {
    debug.printf(DBG_LVL_3, "call Resources::get_block_size\n");

    return block_size;
}

unsigned short Resources::get_preview_block_size() {
    debug.printf(DBG_LVL_3, "call Resources::get_preview_block_size\n");

    return preview_block_size;
}

textures_t * Resources::get_textures() {
    debug.printf(DBG_LVL_3, "call Resources::get_textures\n");

    return &textures;
}

textures_t * Resources::get_preview_textures() {
    debug.printf(DBG_LVL_3, "call Resources::get_preview_textures\n");

    return &preview_textures;
}

levels_t * Resources::get_levels() {
    debug.printf(DBG_LVL_3, "call Resources::get_levels\n");

    return &levels;
}

Level * Resources::get_level(size_t index) {
    debug.printf(DBG_LVL_3, "call Resources::get_level\n");

    return levels[index];
}

palette_t Resources::get_palette() {
    debug.printf(DBG_LVL_3, "call Resources::get_palette\n");

    return palette.get_palette();
}

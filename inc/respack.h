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

#ifndef RESPACK_H
#define RESPACK_H

#include <String.h>
#include <wcvector.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>

#include "debug.h"
#include "exception.h"

typedef enum {
    RECORD_TYPE_FILE = 0x00000001,
    RECORD_TYPE_DIR  = 0x00000002,
} file_record_type_t;

const int       MAX_FILENAME_LEN  = 255;
const int       MAX_PATH_LEN      = MAX_FILENAME_LEN * 3;

const String    MAGICK_CONST("0J/QvtC10LfQtNCwINGD0L3QvtGB0Y/RgiDQsiDQtNCw0LvRjCDRgtCy0L7RjiDQu9GO0LHQvtCy0Yw=");
const String    DATA_BEGIN_SIGNATURE("#PtSokoResPackV0.3");
const String    DATA_END_SIGNATURE("#PtSokoResPackEnd");

typedef struct {
    String              name;
    String              full_path;
    String              real_path;

    uint32_t            type;
    uint32_t            id;
    uint32_t            dir_id;
    uint32_t            offset;
    uint32_t            size;    
    uint32_t            perms;
} file_record_t;

#pragma pack(push, 4)
typedef struct {
    char                name[MAX_FILENAME_LEN];
    char                full_path[MAX_PATH_LEN];

    uint32_t            type;
    uint32_t            id;
    uint32_t            dir_id;
    uint32_t            offset;
    uint32_t            size;    
    uint32_t            perms;
} file_bin_record_t;
#pragma pack(pop)

typedef WCValOrderedVector<file_record_t *>     records_table_t;
typedef WCValOrderedVector<file_bin_record_t *> records_bin_table_t;

//Вещества были забористые
class ResourcePackException : public BaseException {
    public:
        ResourcePackException(String msg) : BaseException(msg) { } 
        ResourcePackException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

class ResourcePack {
    public:
        ResourcePack();

        void        pack(String dir_path, String pack_path);
        void        pack_bin(String bin_path, String fpath);
        void        unpack(String pack_path, String dir_path);
        void        unpack_bin(String bin_path, String dir_path);

    private:
        uint32_t    record_id;
        String      start_path;
        Debug &     debug;

        void        fill_table(records_table_t &table, String path, uint32_t dir_id, uint8_t n = 0);
};

#endif

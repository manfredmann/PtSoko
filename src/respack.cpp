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

#include "respack.h"
#include "help.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

ResourcePack::ResourcePack() : debug(Debug::get_instance()) { }

void ResourcePack::fill_table(records_table_t &table, String path, uint32_t dir_id, uint8_t n) {
    DIR *dir = opendir((const char *) path);

    if (dir == NULL) {
        throw ResourcePackException("Couldn't open dir '%s': %s", (const char *) path, strerror(errno));
    }

    struct dirent * entry = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (!(entry->d_stat.st_status & _FILE_USED)) {
            continue;
        }

        String name(entry->d_name);
        String real_path = path + "/" + name;

        if (name == "." || name == "..") {
            continue;
        }

        if (!S_ISDIR(entry->d_stat.st_mode) && !S_ISREG(entry->d_stat.st_mode)) {
            continue;
        }

        file_record_t *record = new file_record_t;
        memset(record, 0x0, sizeof(file_record_t));

        record->name        = String(entry->d_name);
        record->full_path   = String(real_path, start_path.length() + 1);
        record->real_path   = real_path;
        record->id          = record_id++;
        record->perms       = entry->d_stat.st_mode & S_IPERMS;
        record->dir_id      = dir_id;

        if (S_ISDIR(entry->d_stat.st_mode)) {
            record->type    = RECORD_TYPE_DIR;

            debug.printf(DBG_LVL_1, "Dir:  ");

            for (int i = 0; i < n + 1; ++i) {
                debug.printf(DBG_LVL_1, "  ");
            }

            debug.printf(DBG_LVL_1, " %s\n", (const char *) record->full_path);

            table.insert(record);

            fill_table(table, real_path, record->id);
        } else if (S_ISREG(entry->d_stat.st_mode)) {
            record->type    = RECORD_TYPE_FILE;
            record->size    = entry->d_stat.st_size;

            debug.printf(DBG_LVL_1, "File: ");

            for (int i = 0; i < n + 2; ++i) {
                debug.printf(DBG_LVL_1, "  ");
            }

            debug.printf(DBG_LVL_1, " %s\n", (const char *) record->full_path);

            table.insert(record);
        }
    }
}

void ResourcePack::pack(String dir_path, String pack_path) {
    debug.printf(DBG_LVL_3, "call ResourcePack::pack\n");

    debug.printf(DBG_LVL_1, "Packing...\n");

    records_table_t table;

    record_id   = 0;
    start_path  = dir_path;

    debug.printf(DBG_LVL_1, "Scanning '%s'\n", (const char *) dir_path);

    fill_table(table, dir_path, 0);

    uint32_t records = table.entries();

    if (records == 1) {
        throw ResourcePackException("Directory empty");
    }

    printf("Records: %d\n", records);

    FILE *      res_f;
    uint32_t    i;
    int         r;

    res_f = fopen((const char *) pack_path, "wb");

    if (res_f == NULL) {
        throw ResourcePackException("Couldn't open file '%s': %s", (const char *) pack_path, strerror(errno));
    }

// ==================================

    debug.printf(DBG_LVL_1, "Writing MAGICK_CONST...");

    char *buf = new char[MAGICK_CONST.length() + 1];
    memset(buf, 0x0, MAGICK_CONST.length() + 1);
    strncpy(buf, (const char *) MAGICK_CONST, MAGICK_CONST.length());

    r = fwrite(buf, MAGICK_CONST.length(), 1, res_f);

    delete[] buf;

    if (r != 1) {
        fclose(res_f);
        throw ResourcePackException("Couldn't write resource file");   
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Writing DATA_BEGIN_SIGNATURE...");

    buf = new char[DATA_BEGIN_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_BEGIN_SIGNATURE.length() + 1);
    strncpy(buf, (const char *) DATA_BEGIN_SIGNATURE, DATA_BEGIN_SIGNATURE.length());

    r = fwrite(buf, DATA_BEGIN_SIGNATURE.length(), 1, res_f);

    delete[] buf;

    if (r != 1) {
        fclose(res_f);
        throw ResourcePackException("Couldn't write resource file");   
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Writing records count...");

    r = fwrite(&records, sizeof(records), 1, res_f);

    if (r != 1) {
        fclose(res_f);
        throw ResourcePackException("Couldn't write resource file");   
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================
    
    uint32_t header_offset = ftell(res_f);
    uint32_t data_offset = header_offset + (sizeof(file_bin_record_t) * records);

    fseek(res_f, data_offset, SEEK_SET);

    for (i = 0; i < records; ++i) {
        file_record_t *record = table[i];

        if (record->type == RECORD_TYPE_FILE) {
            record->offset = ftell(res_f);

            if (record->size == 0) {
                continue;
            }

            debug.printf(DBG_LVL_1, "Writing file '%s'...", (const char *) record->full_path);

            FILE *data_f = fopen((const char *) record->real_path, "rb");

            if (data_f == NULL) {
                throw ResourcePackException("Couldn't open file '%s': %s", (const char *) record->real_path, strerror(errno));
            }

            fseek(data_f, 0, SEEK_END);
            size_t data_size = ftell(data_f);
            fseek(data_f, 0, SEEK_SET);

            char *data_buf = new char[data_size];

            fread(data_buf, data_size, 1, data_f);
            fwrite(data_buf, data_size, 1, res_f);

            delete[] data_buf;
            fclose(data_f);

            debug.printf(DBG_LVL_1, "OK\n");
        }
    }

// ==================================

    fseek(res_f, header_offset, SEEK_SET);

    debug.printf(DBG_LVL_1, "Writing records structure...");

    for (i = 0; i < records; ++i) {
        file_record_t *     record = table[i];
        file_bin_record_t   w_record;

        memset(&w_record, 0x0, sizeof(file_bin_record_t));

        strncpy(w_record.name, (const char *) record->name, MAX_FILENAME_LEN);
        strncpy(w_record.full_path, (const char *) record->full_path, MAX_PATH_LEN);
        w_record.type   = record->type;
        w_record.id     = record->id;
        w_record.dir_id = record->dir_id;
        w_record.offset = record->offset;
        w_record.size   = record->size;
        w_record.perms  = record->perms;

        r = fwrite(&w_record, sizeof(file_bin_record_t), 1, res_f);

        if (r != 1) {
            fclose(res_f);
            throw ResourcePackException("Couldn't write resource file");   
        }

    }

    debug.printf(DBG_LVL_1, "OK\n");

    fseek(res_f, 0, SEEK_END);

// ==================================

    debug.printf(DBG_LVL_1, "Writing DATA_END_SIGNATURE...");

    buf = new char[DATA_END_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_END_SIGNATURE.length() + 1);
    strncpy(buf, (const char *) DATA_END_SIGNATURE, DATA_END_SIGNATURE.length());

    r = fwrite(buf, DATA_END_SIGNATURE.length(), 1, res_f);

    delete[] buf;

    if (r != 1) {
        fclose(res_f);
        throw ResourcePackException("Couldn't write resource file");   
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    fclose(res_f);
}

void ResourcePack::unpack(String pack_path, String dir_path) {
    debug.printf(DBG_LVL_3, "call ResourcePack::unpack\n");

    FILE *      bin_f;
    uint32_t    bin_data_offset;
    int         r;

    debug.printf(DBG_LVL_1, "Unpacking...\n");

    bin_f = fopen((const char *) pack_path, "rb");

    if (bin_f == NULL) {
        throw ResourcePackException("Couldn't open file '%s': %s", (const char *) pack_path, strerror(errno));
    }

// ==================================

    debug.printf(DBG_LVL_1, "Reading MAGICK_CONST...");

    char *buf = new char[MAGICK_CONST.length() + 1];
    memset(buf, 0x0, MAGICK_CONST.length() + 1);
 
    r = fread(buf, MAGICK_CONST.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read resource file");
    }

    String magick(buf);

    delete[] buf;

    if (MAGICK_CONST != magick) {
        fclose(bin_f);

        throw ResourcePackException("Incorrect magick signature");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Reading DATA_BEGIN_SIGNATURE...");

    buf = new char[DATA_BEGIN_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_BEGIN_SIGNATURE.length() + 1);

    r = fread(buf, DATA_BEGIN_SIGNATURE.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read resource file");
    }

    String data_begin(buf);

    delete[] buf;

    if (DATA_BEGIN_SIGNATURE != data_begin) {
        printf("%s\n", (const char * ) data_begin);
        throw ResourcePackException("Incorrect data begin signature");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    uint32_t data_offset = ftell(bin_f);

// ==================================

    debug.printf(DBG_LVL_1, "Reading DATA_END_SIGNATURE...");

    fseek(bin_f, -(DATA_END_SIGNATURE.length()), SEEK_END);

    buf = new char[DATA_END_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_END_SIGNATURE.length() + 1);

    r = fread(buf, DATA_END_SIGNATURE.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read resource file");
    }

    String data_end(buf);

    delete[] buf;

    if (data_end != DATA_END_SIGNATURE) {
        fclose(bin_f);

        throw ResourcePackException("Incorrect data end signature");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    fseek(bin_f, data_offset, SEEK_SET);

// ==================================

    debug.printf(DBG_LVL_1, "Reading recrods count...");

    records_bin_table_t table;
    uint32_t            records;
    uint32_t            i;

    r = fread(&records, sizeof(records), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read resource file");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Reading records data...");

    for (i = 0; i < records; ++i) {
        file_bin_record_t *record = new file_bin_record_t;

        r = fread((char *)record, sizeof(file_bin_record_t), 1, bin_f);

        if (r != 1) {
            fclose(bin_f);

            throw ResourcePackException("Couldn't read resource file");
        }

        table.insert(record);
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ===============================

    if (!Help::is_dir_exists(dir_path)) {
        debug.printf(DBG_LVL_1, "Make directory '%s'...", (const char *) dir_path);

        if (mkdir((const char *) dir_path, 0775) != 0) {
            throw ResourcePackException("Couldn't create dir '%s': %s", (const char *) dir_path, strerror(errno));
        }

        debug.printf(DBG_LVL_1, "OK\n");
    }

    for (i = 0; i < table.entries(); ++i) {
        file_bin_record_t *record = table[i];

        String path = dir_path + "/" + String(record->full_path);

        if (record->type == RECORD_TYPE_DIR) {
            debug.printf(DBG_LVL_1, "Make directory '%s'...", (const char *) path);

            if (!Help::is_dir_exists(path)) {
                if (mkdir((const char *) path, record->perms) != 0) {
                    throw ResourcePackException("Couldn't create dir '%s': %s", (const char *) path, strerror(errno));
                }
            }

            debug.printf(DBG_LVL_1, "OK\n");
        }

        if (record->type == RECORD_TYPE_FILE) {
            debug.printf(DBG_LVL_1, "Writing file '%s'...", (const char *) path);

            FILE *data_f = fopen((const char *) path, "wb");

            if (data_f == NULL) {
                throw ResourcePackException("Couldn't open file '%s': %s", (const char *) path, strerror(errno));
            }

            fseek(bin_f, record->offset, SEEK_SET);

            char *  buf = new char[record->size];
            int     r;

            r = fread(buf, 1, record->size, bin_f);

            if (r != record->size) {
                throw ResourcePackException("Read error %d %d", record->size, r);
            }

            r = fwrite(buf, 1, record->size, data_f);

            if (r != record->size) {
                throw ResourcePackException("Write error %d %d", record->size, r);
            }

            fclose(data_f);

            debug.printf(DBG_LVL_1, "OK\n");
        }
    }

    fclose(bin_f);
}

void ResourcePack::unpack_bin(String bin_path, String dir_path) {
    debug.printf(DBG_LVL_3, "call ResourcePack::unpack\n");

    FILE *      bin_f;
    uint32_t    bin_data_offset;
    int         r;

    debug.printf(DBG_LVL_1, "Unpacking...\n");

    bin_f = fopen((const char *) bin_path, "rb");

    if (bin_f == NULL) {
        throw ResourcePackException("Couldn't open file '%s': %s", (const char *) bin_path, strerror(errno));
    }

// ==================================

    debug.printf(DBG_LVL_1, "Reading DATA_END_SIGNATURE...");

    fseek(bin_f, -(sizeof(uint32_t) + DATA_END_SIGNATURE.length()), SEEK_END);

    char *buf = new char[DATA_END_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_END_SIGNATURE.length() + 1);

    r = fread(buf, DATA_END_SIGNATURE.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read binary file");
    }

    String data_end(buf);

    delete[] buf;

    if (data_end != DATA_END_SIGNATURE) {
        fclose(bin_f);

        throw ResourcePackException("Resource pack not found in binary");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Reading data offset...");

    uint32_t pack_data_offset;

    r = fread(&pack_data_offset, sizeof(pack_data_offset), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read binary file");
    }

    fseek(bin_f, pack_data_offset, SEEK_SET);

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Reading MAGICK_CONST...");

    buf = new char[MAGICK_CONST.length() + 1];
    memset(buf, 0x0, MAGICK_CONST.length() + 1);
 
    r = fread(buf, MAGICK_CONST.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read binary file");
    }

    String magick(buf);

    delete[] buf;

    if (MAGICK_CONST != magick) {
        fclose(bin_f);

        throw ResourcePackException("Incorrect magick signature");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Reading MAGICK_CONST...");

    buf = new char[DATA_BEGIN_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_BEGIN_SIGNATURE.length() + 1);

    r = fread(buf, DATA_BEGIN_SIGNATURE.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read binary file");
    }

    String data_begin(buf);

    delete[] buf;

    if (DATA_BEGIN_SIGNATURE != data_begin) {
        throw ResourcePackException("Incorrect data begin signature");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    records_bin_table_t table;
    uint32_t            records;
    uint32_t            i;

    debug.printf(DBG_LVL_1, "Reading recrods count...");

    r = fread(&records, sizeof(records), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read binary file");
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ==================================

    debug.printf(DBG_LVL_1, "Reading records data...");

    for (i = 0; i < records; ++i) {
        file_bin_record_t *record = new file_bin_record_t;

        r = fread((char *)record, sizeof(file_bin_record_t), 1, bin_f);

        if (r != 1) {
            fclose(bin_f);

            throw ResourcePackException("Couldn't read binary file");
        }

        table.insert(record);
    }

    debug.printf(DBG_LVL_1, "OK\n");

// ===============================

    if (!Help::is_dir_exists(dir_path)) {
        debug.printf(DBG_LVL_1, "Make directory '%s'...", (const char *) dir_path);

        if (mkdir((const char *) dir_path, 0755) != 0) {
            throw ResourcePackException("Couldn't create dir '%s': %s", (const char *) dir_path, strerror(errno));
        }

        debug.printf(DBG_LVL_1, "OK\n");
    }

    for (i = 0; i < table.entries(); ++i) {
        file_bin_record_t *record = table[i];

        String path = dir_path + "/" + String(record->full_path);

        if (record->type == RECORD_TYPE_DIR) {
            debug.printf(DBG_LVL_1, "Make directory '%s'...", (const char *) path);

            if (!Help::is_dir_exists(path)) {
                if (mkdir((const char *) path, record->perms) != 0) {
                    throw ResourcePackException("Couldn't create dir '%s': %s", (const char *) path, strerror(errno));
                }
            }

            debug.printf(DBG_LVL_1, "OK\n");
        }

        if (record->type == RECORD_TYPE_FILE) {
            debug.printf(DBG_LVL_1, "Writing file %s...", (const char *) path);

            FILE *data_f = fopen((const char *) path, "wb");

            if (data_f == NULL) {
                throw ResourcePackException("Couldn't open file '%s': %s", (const char *) path, strerror(errno));
            }

            fseek(bin_f, record->offset + pack_data_offset, SEEK_SET);

            char *  buf = new char[record->size];
            int     r;

            r = fread(buf, 1, record->size, bin_f);

            if (r != record->size) {
                throw ResourcePackException("Read error %d %d", record->size, r);
            }

            r = fwrite(buf, 1, record->size, data_f);

            if (r != record->size) {
                throw ResourcePackException("Write error %d %d", record->size, r);
            }

            fclose(data_f);

            debug.printf(DBG_LVL_1, "OK\n");
        }
    }

    fclose(bin_f);
}

void ResourcePack::pack_bin(String bin_path, String fpath) {
    debug.printf(DBG_LVL_3, "call ResourcePack::add_res\n");

    FILE *  res_f;
    size_t  res_size;
    char *  res_buf;
    int     r;

    res_f = fopen((const char *) fpath, "rb");

    if (res_f == NULL) {
        throw ResourcePackException("Couldn't open file '%s': %s", (const char *) fpath, strerror(errno));
    }

    fseek(res_f, 0, SEEK_END);
    res_size = ftell(res_f);
    fseek(res_f, 0, SEEK_SET);

    res_buf = new char[res_size];

    r = fread(res_buf, res_size, 1, res_f);

    if (r != 1) {
        throw ResourcePackException("Couldn't read resource file");
    }

    fclose(res_f);

// ==================================

    char *buf = new char[MAGICK_CONST.length() + 1];

    memset(buf, 0x0, MAGICK_CONST.length() + 1);
    memcpy(buf, res_buf, MAGICK_CONST.length());

    String magick(buf);

    delete[] buf;

    if (MAGICK_CONST != magick) {
        throw ResourcePackException("Incorrect magick signature");
    }

// ==================================

    buf = new char[DATA_BEGIN_SIGNATURE.length() + 1];

    memset(buf, 0x0, DATA_BEGIN_SIGNATURE.length() + 1);
    memcpy(buf, res_buf + MAGICK_CONST.length(), DATA_BEGIN_SIGNATURE.length());

    String data_begin(buf);

    delete[] buf;

    if (DATA_BEGIN_SIGNATURE != data_begin) {
        throw ResourcePackException("Incorrect data begin signature");
    }

// ==================================

    FILE *      bin_f;
    uint32_t    bin_data_offset;

    bin_f = fopen((const char *) bin_path, "a+b");

    if (bin_f == NULL) {
        throw ResourcePackException("Couldn't open file '%s': %s", (const char *) bin_path, strerror(errno));
    }

    bin_data_offset = ftell(bin_f);

    fseek(bin_f, -(sizeof(uint32_t) + DATA_END_SIGNATURE.length()), SEEK_END);

    buf = new char[DATA_END_SIGNATURE.length() + 1];
    memset(buf, 0x0, DATA_END_SIGNATURE.length() + 1);

    r = fread(buf, DATA_END_SIGNATURE.length(), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't read binary file");
    }

    String data_end(buf);

    if (buf == DATA_END_SIGNATURE) {
        fclose(bin_f);

        throw ResourcePackException("Resource pack already in binary");
    }

    fseek(bin_f, bin_data_offset, SEEK_SET);

// ==================================

    r = fwrite(res_buf, res_size, 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't write resource ro binary file");
    }

    r = fwrite(&bin_data_offset, sizeof(bin_data_offset), 1, bin_f);

    if (r != 1) {
        fclose(bin_f);

        throw ResourcePackException("Couldn't write resource ro binary file");
    }

    fclose(bin_f);
}

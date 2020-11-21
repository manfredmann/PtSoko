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

#include "level.h"

#include <string.h>

Level::Level() {
    boxes       = 0;
    box_places  = 0;
    player      = 0;
    width       = 0;
    height      = 0;
    first_run   = true;
}

void Level::load(String fname, String name) {
    FILE *level_file = fopen((const char *)fname, "r");

    if (level_file == NULL) {
        throw LevelException("Couldn't open file '%s'", (const char *) fname);
    }

    this->name = name;

    char line[1024];

    while (fgets(line, sizeof(line), level_file) != NULL) {
        line[strlen(line) - 1] = '\0';

        for (int i = 0; i < strlen(line); ++i) {
            char type = line[i];

            switch (type) {
                case LVL_BTYPE_BRICK: {
                    level += type;
                    break;                      
                }
                case LVL_BTYPE_BOX: {
                    ++boxes;
                    level += type;

                    break;
                }
                case LVL_BTYPE_BOX_PLACE: {
                    ++box_places;
                    level += type;

                    break;
                }
                case LVL_BTYPE_BOXWPLACE: {
                    ++boxes;
                    ++box_places;

                    level += type;
                    break;
                }
                case LVL_BTYPE_PLAYER: {
                    ++player;

                    level += type;
                    break;
                }
                case LVL_BTYPE_EMPTY: {
                    level += ' ';
                    break;
                }
            }
            ++width;
        }

        width = 0;
        ++height;

        level += '\n';
    }

    fclose(level_file);

    check_level_closed();
    fill_level_background();

    memset(&stat, 0x0, sizeof(stat));
}

// Нихера не эффективно, но не пох ли в данном случае? :-)
// Все мы немного Митюк
void Level::fill_level_background() {
    WCValOrderedVector<String> lines;

    size_t  i;
    size_t  j;
    int     k;

    String line;

    for (i = 0; i < level.length(); ++i) {
        char block = level[i];

        if (block == '\n') {
            lines.insert(line);
            line = "";
        } else {
            line += block;
        }
    }

    for (i = 0; i < lines.entries(); ++i) {
        line = lines[i];

        for (j = 0; j < line.length(); ++ j) {
            char block = line[j];

            if (block != ' ') {
                continue;
            }

            bool in_border = false;

            for (k = j; k < line.length(); ++k) {
                if (line[k] == '#') {
                    in_border = true;
                    break;
                }
            }

            if (!in_border) {
                continue;
            }

            in_border = false;

            for (k = j; k >= 0; --k) {
                if (line[k] == '#') {
                    in_border = true;
                    break;
                }
            }

            if (!in_border) {
                continue;
            }

            in_border = false;

            for (k = i; k < lines.entries(); ++k) {
                String tmp_line = lines[k];

                if (tmp_line.length() < j) {
                    continue;
                }

                if (tmp_line[j] == '#') {
                    in_border = true;
                    break;
                }
            }

            if (!in_border) {
                continue;
            }

            in_border = false;

            for (k = i; k >= 0; --k) {
                String tmp_line = lines[k];

                if (tmp_line.length() < j) {
                    continue;
                }

                if (tmp_line[j] == '#') {
                    in_border = true;
                    break;
                }
            }

            if (!in_border) {
                continue;
            }

            lines[i][j] = 'B';
        }
    }


    String level_wback;

    for (i = 0; i < lines.entries(); ++i) {
        line = lines[i];
        level_wback += line;
        level_wback += '\n';
    }

    this->level = level_wback;
}

void Level::check_level_closed() {
    WCValOrderedVector<String> lines;

    size_t  i;
    size_t  j;
    int     k;

    String line;

    for (i = 0; i < level.length(); ++i) {
        char block = level[i];

        if (block == '\n') {
            lines.insert(line);
            line = "";
        } else {
            line += block;
        }
    }

}

String Level::get_level_str() {
    return level;
}

String Level::get_name() {
    return name;
}

unsigned int Level::get_boxes_count() {
    return boxes;
}

unsigned int Level::get_box_places_count() {
    return box_places;
}

unsigned int Level::get_player_count() {
    return player;
}

unsigned int Level::get_width() {
    return width;
}

unsigned int Level::get_height() {
    return height;
}

void Level::stat_reset() {
    stat.time.seconds       = 0;
    stat.time.minutes       = 0;
    stat.moves              = 0;

    stat_best.time.seconds  = 0;
    stat_best.time.minutes  = 0;
    stat_best.moves         = 0;
}

level_stat_t & Level::get_stat() {
    return stat;
}

const level_stat_t & Level::get_stat_best() {
    return stat_best;
}

void Level::set_stat_best() {
    stat_best = stat;
}

void Level::set_stat_best(level_stat_t s) {
    stat_best = s;
}

void Level::save_stat(String fname) {
    // String fname = Help::Sprintf("%s%s.dat", (const char *) home_path, (const char *) lvl->get_name());

    FILE *f = fopen((const char *) fname, "w+");

    if (f == NULL) {
        throw StatisticException("Couldn't open file '%s': %s", (const char *) fname, strerror(errno));
    }

    char    buf[10];
    size_t  r;

    _bprintf(buf, 3, "%d\n", stat_best.time.minutes);

    r = fwrite(buf, 1, 3, f);

    if (r != 3) {
        fclose(f);

        throw StatisticException("Write error. Writed: %d, Needed: %d", r, 3);
    }

    _bprintf(buf, 3, "%d\n", stat_best.time.seconds);

    r = fwrite(buf, 1, 3, f);

    if (r != 3) {
        fclose(f);

        throw StatisticException("Write error. Writed: %d, Needed: %d", r, 3);
    }

    _bprintf(buf, 10, "%d\n", stat_best.moves);

    r = fwrite(buf, 1, 10, f);

    if (r != 10) {
        fclose(f);

        throw StatisticException("Write error. Writed: %d, Needed: %d", r, 3);
    }

    fclose(f);

    first_run = false;
}

void Level::load_stat(String fname) {
    // String fname = Help::Sprintf("%s%s.dat", (const char *) home_path, (const char *) lvl->get_name());

    FILE *f = fopen((const char *) fname, "r");

    if (f == NULL) {
        throw StatisticException("Couldn't open file '%s': %s", (const char *) fname, strerror(errno));
    }

    char    buf[10];
    size_t  r;

    level_stat_t lvl_stat;

    r = fread(buf, 1, 3, f);

    if (r != 3) {
        fclose(f);

        throw StatisticException("Read error. Readed: %d, Needed: %d", r, 3);
    }

    lvl_stat.time.minutes = atoi(buf);

    r = fread(buf, 1, 3, f);

    if (r != 3) {
        fclose(f);

        throw StatisticException("Read error. Readed: %d, Needed: %d", r, 3);
    }

    lvl_stat.time.seconds = atoi(buf);

    r = fread(buf, 1, 10, f);

    if (r != 10) {
        fclose(f);

        throw StatisticException("Read error. Readed: %d, Needed: %d", r, 10);
    }

    lvl_stat.moves = atoi(buf);

    fclose(f);

    stat_best = lvl_stat;
    first_run = false;
}

bool Level::is_first_run() {
    return first_run;
}

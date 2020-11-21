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

#ifndef LEVEL_H
#define LEVEL_H

#include <wcvector.h>
#include <String.h>
#include <bool.h>
#include <stdarg.h>

#include "help.h"
#include "exception.h"

const unsigned    LVL_MAX_BLOCKS_W = 30;
const unsigned    LVL_MAX_BLOCKS_H = 24;

typedef enum {
    LVL_BTYPE_BRICK         = '#',
    LVL_BTYPE_BOX           = '$',
    LVL_BTYPE_BOX_PLACE     = '.',
    LVL_BTYPE_BOXWPLACE     = '*',
    LVL_BTYPE_PLAYER        = '@',
    LVL_BTYPE_EMPTY         = ' ',
    LVL_BTYPE_BACKGROUND    = 'B',
 } level_block_type_t;

typedef struct {
    unsigned int minutes;
    unsigned int seconds;
} level_time_t;

const unsigned int LEVEL_MAX_MOVES = 9999999;

class level_stat_t {
    public:
        level_time_t    time;
        unsigned int    moves;

        friend int operator < (const level_stat_t &a, const level_stat_t &b) {
            return (a.moves < b.moves) && ((a.time.minutes * 60) + a.time.seconds < (b.time.minutes * 60) + b.time.seconds);
        }

        friend int operator > (const level_stat_t &a, const level_stat_t &b) {
            return (a.moves > b.moves) && ((a.time.minutes * 60) + a.time.seconds > (b.time.minutes * 60) + b.time.seconds);
         }

        friend int operator >= (const level_stat_t &a, const level_stat_t &b) {
            return (a.moves >= b.moves) && ((a.time.minutes * 60) + a.time.seconds >= (b.time.minutes * 60) + b.time.seconds);
        }

        friend int operator <= (const level_stat_t &a, const level_stat_t &b) {
            return (a.moves <= b.moves) && ((a.time.minutes * 60) + a.time.seconds <= (b.time.minutes * 60) + b.time.seconds);
        }
};

//Вещества были забористые
class StatisticException : public BaseException {
    public:
        StatisticException(String msg) : BaseException(msg) { } 
        StatisticException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

//Вещества были забористые
class LevelException : public BaseException {
    public:
        LevelException(String msg) : BaseException(msg) { } 
        LevelException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

class Level {
    public:
        Level();

        void                    load(String fname, String name);

        String                  get_level_str();
        String                  get_name();

        unsigned int            get_boxes_count();
        unsigned int            get_box_places_count();
        unsigned int            get_player_count();

        unsigned int            get_width();
        unsigned int            get_height();

        void                    stat_reset();

        level_stat_t &          get_stat();
        const level_stat_t &    get_stat_best();
        void                    set_stat_best();
        void                    set_stat_best(level_stat_t s);

        bool                    is_first_run();

        void                    save_stat(String fname);
        void                    load_stat(String fname);


    private:
        String                  level;
        String                  name;

        unsigned int            boxes;
        unsigned int            box_places;
        unsigned int            player;
        unsigned int            width;
        unsigned int            height;

        level_stat_t            stat;
        level_stat_t            stat_best;

        bool                    first_run;

        void                    fill_level_background();
        void                    check_level_closed();

};

typedef WCValOrderedVector<Level *> levels_t;

#endif

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

#ifndef DEBUG_H
#define DEBUG_H

typedef enum  {
    DBG_LVL_0 = 0,
    DBG_LVL_1 = 1,
    DBG_LVL_2 = 2,
    DBG_LVL_3 = 3,
} debug_level_t;

class Debug {
    public:
        static Debug & get_instance() {
            static Debug debug;

            return debug;
        }

        void set_level(debug_level_t level);
        void printf(debug_level_t level, const char *fmt, ...);

    private:
        Debug();

        debug_level_t level;
};

#endif

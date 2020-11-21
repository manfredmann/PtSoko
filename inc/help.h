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

#ifndef HELP_H
#define HELP_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>

#include <stdio.h>
#include <stdarg.h>
#include <String.h>
#include <bool.h>

class Help {
    public:
        static String             Sprintf(const char *fmt, ...);
        static bool               is_dir_exists(String path);
        static bool               is_file_exists(String path);
        static unsigned int       get_string_width(const char *font, char *str);
        static unsigned int       get_string_height(const char *font, char *str);
        static void               draw_string(unsigned int x, unsigned int y, char *str, char *font, unsigned int color, unsigned int background_color = 0x0, bool back = false);
};

#endif

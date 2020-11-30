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

#include <stdio.h>
#include <optparser.h>
#include <String.h>

#include "game.h"
#include "respack.h"

void signal_handler(int sign) {    
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    OptParser   opt(argv[0], "Sokoban game implementation for QXN4.25/Photon");

    bool        fullscreen  = false;
    String      photon_dev  = "";

    int         width       = 800;
    int         height      = 600;

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGHUP,  signal_handler);

    try {
        String res_list;

        res_list  = "Window size like a\n";
        res_list += "                               640x480 (minimum)\n";
        res_list += "                               800x600 (default)\n";
        res_list += "                               1024x768\n";
        res_list += "                               etc.\n";

        opt.add("s,screen",     "Photon dev path",  OPT_STRING, false, false);
        opt.add("f,fullscreen", "Fullscreen mode");
        opt.add("r,resolution", res_list,           OPT_STRING, false, false);
        opt.add("h,help",       "Help");
        opt.add("u,unpack",     "Unpack resources");
        opt.add("d",            "Debug level",      OPT_COUNTER, false, true);

        if (!opt.parse(argc, argv, true)) {
            return 0;
        }

        if (opt.find("h")) {
            opt.print_help();

            return 0;
        }

        if (opt.find("f")) { 
            fullscreen = true;
        }

        if (opt.find("s")) {
            photon_dev = opt.get_string("s");
        }

        if (opt.find("r")) {
            String r = opt.get_string("r");

            char    sep         = 'x';
            int     sep_index   = 0;
            int     sep_cnt     = 0;

            while ((sep_index = r.index(sep, sep_index + 1)) != -1) {
                sep_cnt++;
            }

            if (sep_cnt == 0 || sep_cnt > 1) {
                opt.print_help();

                return EXIT_FAILURE;
            }

            sep_index = r.index(sep);

            width   = atoi((const char *)String(r, 0, sep_index));
            height  = atoi((const char *)String(r, sep_index + 1));

            if (width < 640 || height < 480) {
                opt.print_help();

                return EXIT_FAILURE;
            }
        }

        Debug &debug = Debug::get_instance();

        if (opt.find("d")) {
            debug.set_level((debug_level_t) opt.get_counter("d"));

        } else {
            debug.set_level(DBG_LVL_0);
        }

        Resources &     res = Resources::get_instance();
        ResourcePack    res_pack;
        String          res_path = res.get_user_home() + String("/.ptsoko/");

        if (opt.find("u")) {
            res_pack.unpack_bin(argv[0], res_path);
        } else {
            bool need_resources = false;

            try {
                res.init();
            } catch (ResourcesException &e) {
                need_resources = true;
            }

            if (need_resources) {
                res_pack.unpack_bin(argv[0], res_path);

                res.init();                
            }

            Game &game = Game::get_instance();

            game.init(photon_dev, fullscreen, width, height);
            game.run();
        }

    } catch (GameException &e) {
        printf("error: %s\n", e.what_c());
    } catch (OptParser_Ex &e) {
        printf("error: %s\n", e.what_c());
    } catch (PaletteException &e) {
        printf("error: %s\n", e.what_c());
    } catch (ResourcesException &e) {
        printf("error: %s\n", e.what_c());
    } catch (ResourcePackException &e) {
        printf("error: %s\n", e.what_c());
    } catch (WCExcept &e) {
        printf("some shit\n");
    }

    return 0;
}

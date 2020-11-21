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

#include "respack.h"

int main(int argc, char *argv[]) {
    OptParser   opt(argv[0], "Sokoban game resource packer for QXN4.25/Photon");
    Debug &     debug = Debug::get_instance();

    debug.set_level(DBG_LVL_1);
    
    try {
        opt.add("p,pack",           "<dir> <res> Pack files from directory to resource file");
        opt.add("u,unpack",         "<res> <dir> Unpack resource file");
        opt.add("b,binary-pack",    "<res> <bin> Add resource pack to binary file");
        opt.add("c,binary-unpack",  "<bin> <dir> Unpack binary");

        if (!opt.parse(argc, argv, true)) {
            return 0;
        }

        if (opt.get_untyped_count() != 2) {
            opt.print_help();

            return EXIT_FAILURE;
        }

        ResourcePack respack;

        if (opt.find("p")) {
            String dir = opt.get_untyped(0);
            String res = opt.get_untyped(1);

            respack.pack(dir, res);
        } else if (opt.find("u")) {
            String res = opt.get_untyped(0);
            String dir = opt.get_untyped(1);

            respack.unpack(res, dir);
        } else if (opt.find("b")) {
            String res = opt.get_untyped(0);
            String bin = opt.get_untyped(1);

            respack.pack_bin(bin, res);  
        } else if (opt.find("c")) {
            String bin = opt.get_untyped(0);
            String dir = opt.get_untyped(1);

            respack.unpack_bin(bin, dir);
        } else {
            opt.print_help();
        }
    } catch (OptParser_Ex &e) {
        printf("error: %s\n", e.what_c());
    } catch (ResourcePackException &e) {
        printf("error: %s\n", e.what_c());
    }

    return 0;
}

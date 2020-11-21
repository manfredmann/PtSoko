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

#ifndef GAME_H
#define GAME_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>

#include <String.h>
#include <wcvector.h>
#include <stdarg.h>

#include <photon/Pf.h>
#include <photon/PhRender.h>
#include <photon/PtLabel.h>

#include "exception.h"
#include "debug.h"
#include "textures.h"
#include "level.h"
#include "state.h"
#include "object.h"
#include "brick.h"
#include "box.h"
#include "box_place.h"
#include "level_background.h"
#include "level_preview.h"
#include "player.h"
#include "help.h"
#include "menu.h"
#include "timer.h"
#include "palette.h"
#include "resources.h"

#define PkIsReleased( f ) ((f & (Pk_KF_Key_Down|Pk_KF_Key_Repeat)) == 0)

typedef WCValOrderedVector<object_pos_t *>  objects_pos_t;
typedef WCValOrderedVector<objects_pos_t *> story_t;

#define GAME_VERSION "0.3.0"

typedef enum {
    MENU_ITEM_LOAD_LVL,
} menu_action_t;

typedef struct {
    menu_action_t   action;
    void *          data;
} menu_item_data_t;

//Вещества были забористые
class GameException : public BaseException {
    public:
        GameException(String msg) : BaseException(msg) { } 
        GameException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

class Game {
    public:
        static  Game& get_instance() {
            static Game instance;
            return instance;
        }

        void                init(String photon_dev, bool fullscreen, int width, int height);
        void                run();

    private:
        Game();
        ~Game();
        
        Game& operator=( Game& );


        Debug &             debug;
        Resources &         res;

        bool                fullscreen;

        unsigned int        block_size;

        String              path_home;
        String              path_stat;

        Menu *              lvl_menu;
        PhDim_t             lvl_menu_size;
        PhPoint_t           lvl_menu_pos;

        PhPoint_t           lvl_preview_pos;
        PhDim_t             lvl_preview_size;

        Level_preview *     lvl_preview;
        textures_t *        lvl_preview_textures;
        unsigned int        lvl_preview_block_size;

        PhDim_t             win_size;
        PtAppContext_t      app;
        PtWidget_t *        window;
        PtWidget_t *        label;
        PtWidget_t *        tim;
        PhImage_t *         buf_draw;
        PmMemoryContext_t * mc;

        objects_t           objects;
        objects_t           boxes;
        objects_t           box_places;
        objects_t           background;

        textures_t *        textures;
        
        game_state_t        state;
        game_state_t        state_pre_menu;

        size_t              level_current;
        story_t             story;

        Timer *             timer;

        palette_t           palette;

        unsigned int        status_height;
        unsigned int        status_font_height;
        char *              status_font;

        bool                clear_screen;

        void                set_state(game_state_t state);

        void                level_load(size_t index);
        void                level_unload();
        void                level_next();
        void                level_prev();
        void                level_restart();
        Level *             level_curr();

        bool                player_move(Player *player, direction_t dir);

        void                draw();

        void                story_add(bool player_only);
        void                story_back();
        void                story_clear();

        void                key_process(unsigned int key);
        
        static int          keyboard_callback(PtWidget_t *widget, void *data, PtCallbackInfo_t *info);
        static int          input_callback(void *data, pid_t rcv_id, void *message, size_t size);

        static void         level_menu_enter_callback(String name, void *data);
        static void         level_menu_select_callback(String name, void *data);

};

#endif

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

#include <String.h>
#include <stdio.h>
#include <time.h>

#include "game.h"

Game::Game() : debug(Debug::get_instance()), res(Resources::get_instance()) {
    debug.printf(DBG_LVL_3, "call Game::Game\n");

    buf_draw                = NULL;
    lvl_menu                = NULL;
    status_font             = NULL;
    mc                      = NULL;
    timer                   = NULL;
    clear_screen            = true;
    block_size              = 0;
    lvl_preview_block_size  = 0;

    path_home               = res.get_path_home();
    path_stat               = res.get_path_stat();
}

Game::~Game() {
    debug.printf(DBG_LVL_3, "call Game::~Game\n");

    if (mc != NULL) {
        PmMemReleaseMC(mc);
    }

    if (lvl_menu != NULL) {
        delete lvl_menu;
    }

    if (lvl_preview != NULL) {
        delete lvl_preview;
    }

    if (buf_draw != NULL) {
        delete buf_draw;
    }

    if (status_font != NULL) {
        delete status_font;
    }
}

void Game::init(String photon_dev, bool fullscreen, int width, int height) {
    debug.printf(DBG_LVL_3, "call Game::init\n");

    PhChannelParms_t    parms       = {0, 0, Ph_DYNAMIC_BUFFER};
    const char *        displayname = NULL;

    if (photon_dev.length() != 0) {
        displayname = photon_dev;
    }

    if (!PhAttach(displayname, &parms)) {
        String err;

        if (displayname) {
            err = Help::Sprintf("Could not attach to Photon manager [%s]", displayname);
        } else if ((displayname = getenv("PHOTON"))) {
            err = Help::Sprintf("Could not attach to Photon manager (PHOTON=[%s]", displayname);
        } else {
            err = String("Could not attach to Photon manager [/dev/photon]");
        }

        throw GameException(err);
    }

    PtInit(NULL);

    PgSetDrawBufferSize(0xFFFF);

// ============================

    short           screen_width;
    short           screen_height;
    unsigned char   screen_n;
    PhRect_t        extent;

// ============================

    this->fullscreen    = fullscreen;

    if (fullscreen) {
        if (PhWindowQueryVisible(Ph_QUERY_GRAPHICS, 0, 1, &extent) == 0) {
           screen_width     = extent.lr.x - extent.ul.x + 1;
           screen_height    = extent.lr.y - extent.ul.y + 1;
           screen_n         = ((extent.lr.x + 1) / screen_width) + (((extent.lr.y) / screen_height) * 3);


           debug.printf(DBG_LVL_1, "Screen width: %d, height: %d, Console number: %d\n", screen_width, screen_height, screen_n);
        } else {
            throw GameException("Couldn't get screen resolution");
        }

        win_size.w = screen_width;
        win_size.h = screen_height;
    } else {
        win_size.w = width;
        win_size.h = height;
    }

// ============================

    PtArg_t         args_win[10];
    PtArg_t         args_lbl[6];

    unsigned int    arg_i   = 0;
    PhArea_t        area    = {0, 0, win_size.w, win_size.h};

    PtSetArg(&args_win[arg_i++], Pt_ARG_MIN_WIDTH,      area.size.w, 0);
    PtSetArg(&args_win[arg_i++], Pt_ARG_MIN_HEIGHT,     area.size.h, 0);
    PtSetArg(&args_win[arg_i++], Pt_ARG_MAX_HEIGHT,     area.size.w, 0);
    PtSetArg(&args_win[arg_i++], Pt_ARG_MAX_WIDTH,      area.size.h, 0);
    PtSetArg(&args_win[arg_i++], Pt_ARG_WINDOW_TITLE,   "Sokoban", 0);

    if (!fullscreen) {
        PtSetArg(&args_win[arg_i++], Pt_ARG_WINDOW_RENDER_FLAGS,
            Ph_WM_RENDER_ASAPP |
            Ph_WM_RENDER_CLOSE |
            Ph_WM_RENDER_TITLE |
            Ph_WM_RENDER_MIN,
            Pt_TRUE);

    } else {        
        PtSetArg(&args_win[arg_i++], Pt_ARG_WINDOW_RENDER_FLAGS,
            Ph_WM_RENDER_ASAPP,
            Pt_TRUE);
    }

    PtSetArg(&args_win[arg_i++], Pt_ARG_WINDOW_CURSOR_OVERRIDE, Pt_TRUE, 0);
    PtSetArg(&args_win[arg_i++], Pt_ARG_WINDOW_STATE,           Ph_WM_STATE_ISFRONT, 0);
    PtSetArg(&args_win[arg_i++], Pt_ARG_CURSOR_TYPE,            Ph_CURSOR_NONE, 0);

    PtSetParentWidget(NULL);
    window = PtCreateWidget(PtWindow, NULL, arg_i, args_win);

    PtAddEventHandler(window, Ph_EV_KEY, &keyboard_callback, NULL);

// ============================

    buf_draw        = new PhImage_t;
    buf_draw->type  = Pg_IMAGE_DIRECT_888;
    buf_draw->size  = area.size;
    buf_draw->image = new char[area.size.w * area.size.h * 3];
    arg_i           = 0;

    PtSetArg(&args_lbl[arg_i++], Pt_ARG_LABEL_TYPE,     Pt_IMAGE, 0 );
    PtSetArg(&args_lbl[arg_i++], Pt_ARG_AREA,           &area, 0 );
    PtSetArg(&args_lbl[arg_i++], Pt_ARG_LABEL_DATA,     buf_draw, sizeof(*buf_draw));
    PtSetArg(&args_lbl[arg_i++], Pt_ARG_MARGIN_HEIGHT,  0, 0);
    PtSetArg(&args_lbl[arg_i++], Pt_ARG_MARGIN_WIDTH,   0, 0);
    PtSetArg(&args_lbl[arg_i++], Pt_ARG_BORDER_WIDTH,   0, 0);

    label = PtCreateWidget(PtLabel, window, arg_i, args_lbl);

// ============================

    PhPoint_t translation = { 0, 0 };
    mc = PmMemCreateMC(buf_draw, &win_size, &translation);

// ============================

    state               = STATE_INIT;
    level_current       = 0;

// ============================

    status_font         = "pcterm14";
    status_font_height  = Help::get_string_height(status_font, "Status");
    status_height       = status_font_height + 10;

// ============================

    res.load_levels();

// ============================

    lvl_menu_size.w = (win_size.w / 100) * 15;
    lvl_menu_size.h = win_size.h;

    lvl_menu_pos.x = 10;
    lvl_menu_pos.y = 0;

    lvl_menu = new Menu(lvl_menu_size, lvl_menu_pos, palette);
    lvl_menu->add_enter_callback(Game::level_menu_enter_callback);
    lvl_menu->add_select_callback(Game::level_menu_select_callback);

    for (size_t i = 0; i < res.get_levels()->entries(); ++i) {
        Level *lvl = res.get_level(i);

        menu_item_data_t *MenuItem = new menu_item_data_t;

        MenuItem->action            = MENU_ITEM_LOAD_LVL;
        MenuItem->data              = new size_t;
        * (size_t *) MenuItem->data = i;
        
        lvl_menu->add_item(lvl->get_name(), MenuItem);
    }

// ============================

    lvl_preview_pos.x = 10 + lvl_menu_size.w;
    lvl_preview_pos.y = 0;

    lvl_preview_size.w = win_size.w - lvl_preview_pos.x;
    lvl_preview_size.h = win_size.h;

// ============================

    res.load_palette();
    res.load_textures(win_size, lvl_preview_size);

    palette = res.get_palette();

    block_size              = res.get_block_size();
    textures                = res.get_textures();
    lvl_preview_block_size  = res.get_preview_block_size();
    lvl_preview_textures    = res.get_preview_textures();

//  ============================

    lvl_preview = new Level_preview(lvl_preview_pos, lvl_preview_size, lvl_preview_block_size, *lvl_preview_textures, palette);

// ============================

    PtRealizeWidget(window);
    PtWindowToFront(window);
    PtWindowFocus(window);

    app = PtDefaultAppContext();

    PtAppAddInput(app, 0, Game::input_callback, NULL);

// ============================

    lvl_menu->select(0);

    set_state(STATE_SPLASH);
}

void Game::run() {
    debug.printf(DBG_LVL_3, "call Game::run\n");

    PtMainLoop();
}

bool check(object_pos_t r1, object_pos_t r2) {
    int x0 = r1.x;
    int y0 = r1.y;

    int x1 = r1.x + r1.w;
    int y1 = r1.y + r1.h;

    int x2 = r2.x;
    int y2 = r2.y;

    int x3 = r2.x + r2.w;
    int y3 = r2.y + r2.h;

    if (x0 > x3 || x1 < x2 || y0 > y3 || y1 < y2 ) {
        return false;
    } 
    return true;
}

void Game::story_add(bool player_only) {
    debug.printf(DBG_LVL_3, "call Game::story_add\n");

    objects_pos_t *positions = new objects_pos_t;

    if (!player_only) {
        for (size_t i = 0; i < boxes.entries(); ++i) {
            Object *obj         = boxes[i];
            object_pos_t *pos   = new object_pos_t;

            *pos                = obj->get_pos();

            positions->insert(pos);
        }       
    }

    Object *obj = objects[objects.entries() - 1];

    object_pos_t *pos   = new object_pos_t;
    *pos                = obj->get_pos();
    
    positions->insert(pos);

    story.insert(positions);
}

void Game::story_back() {
    debug.printf(DBG_LVL_3, "call Game::story_back\n");

    if (story.entries() == 0) {
        return;
    }

    size_t          i;
    objects_pos_t * last_move = story[story.entries() - 1];

    for (i = 0; i < background.entries(); ++i) {
        Object *obj = background[i];
        obj->set_changed();
    }

    for (i = 0; i < box_places.entries(); ++i) {
        Object *obj = box_places[i];
        obj->set_changed();
    }

    for (i = 0; i < boxes.entries(); ++i) {
        Object *obj = boxes[i];
        obj->set_changed();
    }

    //Restore Box positions
    if (last_move->entries() > 1) {
        for (size_t i = 0; i < last_move->entries() - 1; ++i) {
            object_pos_t *pos = (*last_move)[i];
            Object *obj = boxes[i];

            obj->set_pos(pos->x, pos->y);
            obj->set_changed();
        }
    }

    //Restore Player position
    object_pos_t *pos = (*last_move)[last_move->entries() - 1];
    Object *obj = objects[objects.entries() - 1];

    obj->set_pos(pos->x, pos->y);
    obj->set_changed();

    delete last_move;
    story.removeLast();

    level_curr()->get_stat().moves--;
}

void Game::story_clear() {
    debug.printf(DBG_LVL_3, "call Game::story_clear\n");

    while(story.entries() != 0) {
        objects_pos_t *moves = story.last();

        while (moves->entries() != 0) {
            delete moves->last();
            moves->removeLast();
        }
        
        delete story.last();
        story.removeLast();
    }
}

bool Game::player_move(Player *player, direction_t dir) {
    debug.printf(DBG_LVL_3, "call Game::player_move\n");

    object_pos_t    player_pos_next = player->move_next(dir);
    object_pos_t    player_pos      = player->get_pos();
    Box *           box             = NULL;
    size_t          i;
    bool            is_move_correct = true;

    for (i = 0; i < objects.entries(); ++i) {
        Object *        object  = objects[i];
        object_pos_t    obj_pos = object->get_pos();

        switch(object->get_type()) {
            case OBJECT_BRICK: {
                if (check(player_pos_next, obj_pos)) {
                    is_move_correct = false;
                }
                break;
            }
            case OBJECT_BOX: {
                if (check(player_pos_next, obj_pos)) {
                    box = (Box *)object;
                }
                break;
            }
            default: continue;
        }
    }

    if (is_move_correct) {
        // for (i = 0; i < boxes.entries(); ++i) {
        //     Box *box = (Box *) boxes[i];

        //     for (size_t j = 0; j < box_places.entries(); ++j) {
        //         Box_place *box_place = (Box_place *) box_places[j];

        //         if (check(box->get_pos(), box_place->get_pos())) {
        //             box->set_changed();
        //         }
        //     }
        // }

        if (box != NULL) {
            object_pos_t box_pos_next = box->move_calc(dir);

            for (size_t i = 0; i < objects.entries(); ++i) {

                Object *        object  = objects[i];
                object_pos_t    obj_pos = object->get_pos();

                switch(object->get_type()) {
                    case OBJECT_BOX: {
                        if (((Box *) object) == box) {
                            continue;
                        }
                    }
                    case OBJECT_BRICK:{
                        if (check(box_pos_next, obj_pos)) {
                            return false;
                        }
                        break;
                    }
                }
            }

            story_add(false);
            box->move(dir);
        } else {
            story_add(true);
        }

        for (size_t i = 0; i < objects.entries(); ++i) {
            Object *obj = (Object *) objects[i];

            if (obj->get_type() == OBJECT_BACKGROUND || obj->get_type() == OBJECT_BOXPLACE) {
                if (obj->get_pos().x == player_pos.x &&
                    obj->get_pos().y == player_pos.y) {

                    obj->set_changed();
                    break;
                }                
            }   
        }

        player->move(dir);

        level_curr()->get_stat().moves++;

        for (i = 0; i < boxes.entries(); ++i) {
            Box *box = (Box *) boxes[i];

            bool find = false;
            for (size_t j = 0; j < box_places.entries(); ++j) {
                Box_place *box_place = (Box_place *) box_places[j];

                if (check(box->get_pos(), box_place->get_pos())) {
                    box->set_changed();

                    find = true;
                    break;
                }
            }

            if (find == false) {
                return is_move_correct;
            }
        }

        // =============== WIN!!! =================

        Level *lvl = level_curr();

        level_stat_t &          level_stat         = lvl->get_stat();
        const level_stat_t &    level_stat_best    = lvl->get_stat_best();

        if (lvl->is_first_run() || level_stat <= level_stat_best) {
            lvl->set_stat_best();
    
            try {
                String stat_fname = Help::Sprintf("%s/%s.dat", (const char *) path_stat, (const char *) lvl->get_name());

                lvl->save_stat(stat_fname);
            } catch (StatisticException &e) {
                debug.printf(DBG_LVL_1, "Couldn't save statistic: %s\n", e.what_c());
            }
        }

        level_next();
    }

    return is_move_correct;
}

void Game::key_process(unsigned int key) {
    debug.printf(DBG_LVL_3, "call Game::key_process\n");

    bool need_draw = false;

    switch(state) {
        case STATE_INIT:
        case STATE_LOADING: 
        case STATE_WIN: {
            break;
        }
        case STATE_MENU: {
            switch(key) {
                case Pk_Escape:
                case Pk_m: {
                    set_state(state_pre_menu);

                    break;
                }

                case Pk_Up: {
                    lvl_menu->up();

                    need_draw = true;
                    break;
                }

                case Pk_Down: {
                    lvl_menu->down();
                    
                    need_draw = true;
                    break;
                }

                case Pk_Left: {
                    lvl_menu->left();

                    need_draw = true;
                    break;
                }
                case Pk_Right: {
                    lvl_menu->right();

                    need_draw = true;
                    break;
                }
                case Pk_Return: {
                    lvl_menu->enter();

                    break;
                }
            }

            break;
        }
        case STATE_SPLASH: {
            switch(key) {
                case Pk_s: {
                    level_current = 0;
                    level_load(level_current);

                    break;
                }
                case Pk_m: {
                    state_pre_menu = state;

                    lvl_menu->select(level_current);
                    set_state(STATE_MENU);

                    break;
                }
                case Pk_Escape: {
                    exit(EXIT_SUCCESS);
                }
            }

            break;
        }
        case STATE_END: {
            switch(key) {
                case Pk_Escape: {
                    exit(EXIT_SUCCESS);
                }
                case Pk_m: {
                    state_pre_menu = state;

                    lvl_menu->select(level_current);
                    set_state(STATE_MENU);

                    break;
                }
                case Pk_s: {
                    level_current = 0;

                    level_load(level_current);

                    break;
                }
            }

            break;
        }
        case STATE_GAME: {
            Player *player = NULL;

            for (size_t i = 0; i < objects.entries(); ++i) {
                if (objects[i]->get_type() == OBJECT_PLAYER) {
                    player = (Player *) objects[i];
                    break;
                }
            }

            if (player == NULL) {
                throw GameException("Player == NULL in draw method at STATE_GAME");
            }
            
            switch(key) {
                case Pk_m: {
                    state_pre_menu = STATE_GAME;

                    lvl_menu->select(level_current);
                    set_state(STATE_MENU);
                    
                    break;
                }
                case Pk_Up: {
                    if (player_move(player, DIRECTION_UP)) {
                        need_draw = true;
                    }

                    break;
                }
                case Pk_Down: {
                    if (player_move(player, DIRECTION_DOWN)) {
                        need_draw = true;
                    }

                    break;
                }
                case Pk_Left: {
                    if (player_move(player, DIRECTION_LEFT)) {
                        need_draw = true;
                    }

                    break;
                }
                case Pk_Right: {
                    if (player_move(player, DIRECTION_RIGHT)) {
                        need_draw = true;                       
                    }

                    break;
                }
                case Pk_BackSpace: {
                    story_back();
                    need_draw = true;

                    break;
                }
                case Pk_r: {
                    level_restart();

                    break;
                }
                case Pk_n: {
                    level_next();

                    break;
                }
                case Pk_p: {
                    level_prev();

                    break;
                }
                case Pk_Escape: {
                    exit(EXIT_SUCCESS);
                }
            }
            break;
        }
    }

    if (need_draw) {
        draw();
    }
}

void Game::draw() {
    debug.printf(DBG_LVL_3, "call Game::draw\n");

    PmMemStart(mc);

    if (clear_screen) {
        PgSetFillColor(palette.background);
        PgDrawIRect(0, 0, win_size.w, win_size.h, Pg_DRAW_FILL );
        clear_screen = false;

        for (size_t i = 0; i < objects.entries(); ++i) {
            Object *object = objects[i];
            object->set_changed();
        }
    }

    switch(state) {
        case STATE_SPLASH: {
            debug.printf(DBG_LVL_3, "Draw STATE_SPLASH\n");

            char            str[100];
            char *          splash_font = "pcterm14";
            unsigned int    h;
            int             s;
            unsigned int    x;
            unsigned int    y;

            sprintf(str, "Sokoban for QNX4.25/Photon v%s", GAME_VERSION);
            h = Help::get_string_height(splash_font, str);

            x = h / 2;
            y = 0;
            s = 1;

            Help::draw_string(x, y + h + s, str, splash_font, palette.font);

            _bprintf(str, 100, "Control: ");
            Help::draw_string(x, y + (h * 2) + s, str, splash_font, palette.font);

            _bprintf(str, 100, "N - Next level");
            Help::draw_string(x, y + (h * 3) + s, str, splash_font, palette.font);

            _bprintf(str, 100, "P - Previous level");
            Help::draw_string(x, y + (h * 4) + s, str, splash_font, palette.font);

            _bprintf(str, 100, "R - Restart level");
            Help::draw_string(x, y + (h * 5) + s, str, splash_font, palette.font);

            _bprintf(str, 100, "M - Level selection");
            Help::draw_string(x, y + (h * 6) + s, str, splash_font, palette.font);

            _bprintf(str, 100, "Backspace - Undo");
            Help::draw_string(x, y + (h * 7) + s, str, splash_font, palette.font);

            _bprintf(str, 100, "Press S to start");
            Help::draw_string((win_size.w / 2) - (Help::get_string_width(splash_font, str) / 2), 
                (win_size.h / 2), 
                str, 
                splash_font, 
                palette.font);

            _bprintf(str, 100, "(C) %s 2019-2020", "Roman Serov");
            Help::draw_string(win_size.w - Help::get_string_width(splash_font, str) - (h / 2), 
                win_size.h - (h / 2), 
                str, 
                splash_font, 
                palette.font);

            break;
        }
        case STATE_INIT: {
            debug.printf(DBG_LVL_3, "Draw STATE_INIT\n");
            break;
        }
        case STATE_LOADING: {
            debug.printf(DBG_LVL_3, "Draw STATE_LOADING\n");
            break;
        }
        case STATE_END: {
            debug.printf(DBG_LVL_3, "Draw STATE_END\n");

            char str[8];

            _bprintf(str, 8, "The END");

            Help::draw_string((win_size.w / 2) - (Help::get_string_width("pcterm20", str) / 2), (win_size.h / 2), str, "pcterm20", palette.font);
            break;
        }
        case STATE_GAME: {
            debug.printf(DBG_LVL_3, "Draw STATE_GAME\n");

            Level *lvl = level_curr();

            PhPoint_t p;
            
            char status_level_name[50];
            char status_level_info[100];
            char status_level_best_info[100];

            level_stat_t &          level_stat       = lvl->get_stat();
            const level_stat_t &    level_best_stat  = lvl->get_stat_best();

            _bprintf(status_level_name, 50, "%s", (const char *) lvl->get_name());
            _bprintf(status_level_info, 100, "%02d:%02d / %-4d", 
                level_stat.time.minutes, 
                level_stat.time.seconds, 
                level_stat.moves);

            PgSetFillColor(palette.status_background);
            PgDrawIRect(0, win_size.h - status_height, win_size.w, win_size.h, Pg_DRAW_FILL);

            // =======================================

            p.x = (status_height / 2) + 1;
            p.y = win_size.h - (status_height / 2) + (status_font_height / 3);

            Help::draw_string(p.x, p.y, status_level_name, status_font, palette.status_font);

            // =======================================

            p.x = (win_size.w / 2) - (Help::get_string_width(status_font, status_level_info) / 2);

            Help::draw_string(p.x, p.y, status_level_info, status_font, palette.status_font);

            // =======================================

            if (!lvl->is_first_run()) {
                _bprintf(status_level_best_info, 100, "%02d:%02d / %4d", 
                    level_best_stat.time.minutes, 
                    level_best_stat.time.seconds, 
                    level_best_stat.moves);
            } else {
                _bprintf(status_level_best_info, 100, "%s", "First run");                 
            }

            p.x = win_size.w - Help::get_string_width(status_font, status_level_best_info) - ((status_height / 2) + 1);

            Help::draw_string(p.x, p.y, status_level_best_info, status_font, palette.status_font);

            // =======================================

            for (size_t i = 0; i < objects.entries(); ++i) {
                Object *object = objects[i];

                object->draw();
            }

            break;
        }
        case STATE_MENU: {
            debug.printf(DBG_LVL_3, "Draw STATE_MENU\n");

            if (lvl_menu != NULL && lvl_preview != NULL) {
                lvl_menu->draw();
                lvl_preview->draw();
            }

            break;
        }
    }

    PmMemFlush(mc, buf_draw);
    PmMemStop(mc);

    PtArg_t args[1];
    PtSetArg(&args[0], Pt_ARG_LABEL_DATA, buf_draw, sizeof(*buf_draw));
    PtSetResources(label, 1, args);
}

int Game::keyboard_callback(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) {
    Game & game    = Game::get_instance();

    // game.debug.printf(DBG_LVL_3, "call Game::keyboard_callback\n");

    if (info->event->type == Ph_EV_KEY) {
        PhKeyEvent_t *  ke      = (PhKeyEvent_t *) PhGetData(info->event);

        if (PkIsFirstDown(ke->key_flags)) {
            game.key_process(ke->key_cap);
        } else if (PkIsReleased(ke->key_flags)) {
            //game->key_process(ke->key_cap, false, true);
        } else {
            game.key_process(ke->key_cap);
        }
    }
    return Pt_CONTINUE;
}

int Game::input_callback(void *data, pid_t rcv_id, void *message, size_t size) {
    Game & game    = Game::get_instance();

    if (game.timer != NULL) {
        if (rcv_id == game.timer->get_proxy() && game.state == STATE_GAME) {
             level_stat_t & level_stat = game.level_curr()->get_stat();

            if (level_stat.time.seconds + 1 == 60) {
                level_stat.time.minutes++;
                level_stat.time.seconds = 0;
            } else {
                level_stat.time.seconds++;
            }

            game.draw();
        }
    }

    return Pt_CONTINUE;
}

void Game::set_state(game_state_t state) {
    debug.printf(DBG_LVL_3, "call Game::set_state\n");

    this->state         = state;
    this->clear_screen  = true;

    debug.printf(DBG_LVL_2, "Set state: %d\n", state);

    draw();
}

Level *Game::level_curr() {
    debug.printf(DBG_LVL_3, "call Game::level_curr\n");

    return res.get_level(level_current);
}

void Game::level_restart() {
    debug.printf(DBG_LVL_3, "call Game::level_restart\n");

    level_unload();
    level_load(level_current);
}

void Game::level_next() {
    debug.printf(DBG_LVL_3, "call Game::level_next\n");

    level_unload();

    level_current++;

    if (level_current < res.get_levels()->entries()) {
        level_load(level_current);
    } else {
        set_state(STATE_END);
    }
}

void Game::level_prev() {
    debug.printf(DBG_LVL_3, "call Game::level_prev\n");

    level_unload();

    if (level_current >= 1) {
        level_load(--level_current);
    } else {
        set_state(STATE_SPLASH);
    }
}

void Game::level_load(size_t index) {
    debug.printf(DBG_LVL_3, "call Game::level_load\n");

    set_state(STATE_LOADING);

    Level * lvl         = level_curr();
    String  level_str   = lvl->get_level_str();

    size_t          i;

    unsigned int    x           = 0;
    unsigned int    y           = 0;
    
    unsigned int    x_max       = 0;
    unsigned int    y_max       = 0;
    
    unsigned int    x_offset    = 0;
    unsigned int    y_offset    = 0;

    unsigned int    step        = block_size;

    Player *        player      = NULL;

    for (i = 0; i < level_str.length(); ++i) {
        char type = level_str[i];

        switch(type) {
            case LVL_BTYPE_BRICK: {
                Brick *brick = new Brick(x, y, step - 1, step - 1, textures->brick);

                objects.insert((Object *)brick);

                break;
            }
            case LVL_BTYPE_BOX: {
                Level_background *  lvl_back = new Level_background(x, y, step - 1, step - 1, palette.level_background);
                Box *               box = new Box(x, y, step - 1, step - 1, textures->box);

                boxes.insert((Object *)box);
                background.insert((Object *) lvl_back);

                break;
            }
            case LVL_BTYPE_BOX_PLACE: {
                Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures->box_place);

                box_places.insert((Object *)box_place);

                break;
            }
            case LVL_BTYPE_BOXWPLACE: {
                Box *box = new Box(x, y, step - 1, step - 1, textures->box);
                Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures->box_place);

                boxes.insert((Object *)box);
                box_places.insert((Object *)box_place);

                break;
            }
            case LVL_BTYPE_PLAYER: {
                player = new Player(x, y, step - 1, step - 1);

                Level_background *lvl_back = new Level_background(x, y, step - 1, step - 1, palette.level_background);
                background.insert((Object *) lvl_back);

                break;
            }
            case LVL_BTYPE_BACKGROUND: {
                Level_background *lvl_back = new Level_background(x, y, step - 1, step - 1, palette.level_background);

                background.insert((Object *) lvl_back);

                break;
            }
            case '\n': {
                y += step;

                if (y_max < y) {
                    y_max = y;
                }

                x = 0;
                continue;
            }
        }

        x += step;

        if (x_max < x) {
            x_max = x;
        }
    }

    x_offset = (win_size.w - x_max) / 2;
    y_offset = (win_size.h - y_max - status_height) / 2;

    for (i = 0; i < background.entries(); ++i) {
        objects.insert(background[i]);
    }

    for (i = 0; i < box_places.entries(); ++i) {
        objects.insert(box_places[i]);
    }

    for (i = 0; i < boxes.entries(); ++i) {
        objects.insert(boxes[i]);
    }

    objects.insert((Object *)player);

    for (i = 0; i < objects.entries(); ++i) {
        Object *obj = objects[i];

        object_pos_t obj_pos = obj->get_pos();
        
        obj_pos.x += x_offset;
        obj_pos.y += y_offset;

        obj->set_pos(obj_pos.x, obj_pos.y);
        obj->set_changed();
    }

    lvl->stat_reset();

    try {
        String stat_fname = Help::Sprintf("%s/%s.dat", (const char *) path_stat, (const char *) lvl->get_name());

        lvl->load_stat(stat_fname);
    } catch (StatisticException &e) {
        debug.printf(DBG_LVL_1, "Couldn't load statistic file (first run?): %s\n", e.what_c());
    }

    timer = new Timer(1, 0);

    set_state(STATE_GAME);
}

void Game::level_unload() {
    debug.printf(DBG_LVL_3, "call Game::level_unload\n");
    
    delete timer;
    timer = NULL;

    set_state(STATE_INIT);

    story_clear();

    while (objects.entries() != 0) {
        Object *obj = objects.first();

        switch(obj->get_type()) {
            case OBJECT_BRICK: {
                delete (Brick *) obj;
                break;
            }
            case OBJECT_BOX: {
                delete (Box *) obj;
                break;
            }
            case OBJECT_BOXPLACE: {
                delete (Box_place *) obj;
                break;
            }
            case OBJECT_PLAYER: {
                delete (Player *) obj;
                break;
            }
            case OBJECT_BACKGROUND: {
                delete (Level_background *) obj;
                break;              
            }
        }

        objects.removeFirst();
    }


    while (background.entries() != 0) {
        background.removeFirst();
    }

    while (boxes.entries() != 0) {
        boxes.removeFirst();
    }

    while (box_places.entries() != 0) {
        box_places.removeFirst();
    }
}

void Game::level_menu_enter_callback(String name, void *data) {
    menu_item_data_t * menu_item   = (menu_item_data_t *) data;
    Game &             game        = Game::get_instance();

    game.debug.printf(DBG_LVL_3, "call Game::level_menu_enter_callback\n");

    switch(menu_item->action) {
        case MENU_ITEM_LOAD_LVL: {
            size_t level_index = *((size_t *) menu_item->data);

            game.level_unload();
            game.level_current = level_index;
            game.level_load(level_index);

            break;
        }
    }
}

void Game::level_menu_select_callback(String name, void *data) {
    menu_item_data_t * menu_item  = (menu_item_data_t *) data;
    Game &             game       = Game::get_instance();

    game.debug.printf(DBG_LVL_3, "call Game::level_menu_select_callback\n");

    switch(menu_item->action) {
        case MENU_ITEM_LOAD_LVL: {
            size_t  level_index = *((size_t *) menu_item->data);
            Level * lvl         = game.res.get_level(level_index);

            game.lvl_preview->load_level(lvl);

            break;
        }
    }   
}

/*
* Copyright (C) 2019 Roman Serov <roman@serov.co>
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

#include <dirent.h>
#include <String.h>
#include <wcvector.h>

#include <photon/Pf.h>
#include <photon/PhRender.h>
#include <photon/PtLabel.h>

#include "object.h"
#include "brick.h"
#include "box.h"
#include "box_place.h"
#include "player.h"

#define PkIsReleased( f ) ((f & (Pk_KF_Key_Down|Pk_KF_Key_Repeat)) == 0)

typedef	WCValOrderedVector<object_pos_t *>	objects_pos_t;
typedef WCValSortedVector<String>			levels_t;
typedef WCValOrderedVector<Object *>		objects_t;
typedef WCValOrderedVector<objects_pos_t *>	story_t;

#define GAME_AUTHOR		"Roman Serov"
#define GAME_VERSION	0.1

typedef enum {
	STATE_INIT,
	STATE_SPLASH,
	STATE_LOADING,
	STATE_GAME,
	STATE_WIN,
	STATE_END,
} game_state_t;

typedef enum {
	LVL_BTYPE_BRICK 	= '#',
	LVL_BTYPE_BOX		= '$',
	LVL_BTYPE_BOX_PLACE = '.',
	LVL_BTYPE_BOXWPLACE	= '*',
	LVL_BTYPE_PLAYER	= '@',
	LVL_BTYPE_EMPTY		= ' ',
} level_block_type_t;

typedef struct {
	PhImage_t * box;
	PhImage_t * box_place;
	PhImage_t * brick;
} textures_t;

class Game_ex {
	public:
		Game_ex(String msg) {
			this->msg = msg;
		}

		String what() {
			return this->msg;
		}


	private:
		String msg;
};

class Game {
	public:
		static 	Game& get_instance() {
			static Game instance;
			return instance;
		}

		void 				init();
		void				run();

	private:
		Game();
		Game& operator=( Game& );

		unsigned int		block_h;
		unsigned int		block_w;

		unsigned int		blocks_w;
		unsigned int 		blocks_h;

		String 				soko_home[3];

		PhDim_t				win_size;
		PtAppContext_t		app;
		PtWidget_t *		window;
		PtWidget_t * 		label;
		PtWidget_t *		tim;
		PhImage_t *			buf_draw;
		PmMemoryContext_t *	mc;
		objects_t			objects;

		objects_t			boxes;
		objects_t			box_places;

		textures_t			textures;
		game_state_t		state;
		levels_t			levels;

		size_t				level_current;
		story_t				story;

		unsigned int		moves;
		unsigned int		status_height;
		char *				status_font;

		void 				set_state(game_state_t state);

		String				texture_find(String path);

		void				level_load(size_t index);
		void				level_unload();
		void 				level_next();
		void 				level_prev();
		void				level_restart();
		String				level_name();
		void 				player_move(Player *player, direction_t dir);
		void				draw();
		void 				draw_string(unsigned int x, unsigned int y, char *str, char *font, unsigned int color);

		unsigned int		get_string_width(char *font, char *str);
		unsigned int		get_string_height(char *font, char *str);

		void				story_add(bool player_only);
		void				story_back();
		void				story_clear();

		void 				key_process(unsigned int key);
		
		static int			keyboard_callback(PtWidget_t *widget, void *data, PtCallbackInfo_t *info);
};

#endif
/*
* PtSoko - Sokoban for QNX4.25/Photon
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

#include "game.h"
#include <String.h>
#include <stdio.h>
#include <time.h>

static char *img_mem;

static void *img_memory_allocate(long nbytes, int type) {
	if( type == PX_IMAGE ) {
		return PgShmemCreate( nbytes, NULL);
	} else {
		return calloc(1, nbytes);
	}
}

static void *img_memory_free(void *memory, int type) {
	if (type == PX_IMAGE) {
		PgShmemDestroy(memory);
	} else {
		free(memory);
	}

	return NULL;
}

static void *img_warning(char *msg) {
	printf("%s\n", msg);
	return NULL;
}

static void *img_error(char *msg) {
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
	return NULL;
}

static void *img_progress(int percent) {
	//printf("Texture load status:  %d.%d percent\n", percent >> 16, percent & 0xffff);
	return NULL;
}

Game::Game() {
	PhChannelParms_t 	parms		= {0, 0, Ph_DYNAMIC_BUFFER};
	char *				displayname	= NULL;

	if (!PhAttach(displayname, &parms)) {
		String err;

		if (displayname) {
			err = Help::Sprintf("Could not attach to Photon manager [%s]", displayname);
		} else if ((displayname = getenv("PHOTON"))) {
			err = Help::Sprintf("Could not attach to Photon manager (PHOTON=[%s]", displayname);
		} else {
			err = String("Could not attach to Photon manager [/dev/photon]");
		}

		throw Game_ex(err);
	}

	PtInit(NULL);

	PgSetDrawBufferSize(0xFFFF);

	block_h = GAME_BLOCK_SIZE;
	block_w = GAME_BLOCK_SIZE;

	blocks_w = 31;
	blocks_h = 24;

	win_size.w = blocks_w * block_w;
	win_size.h = (blocks_h * block_h) + 24;

	PtArg_t		args_win[10];
	PtArg_t		args_lbl[6];

	PhArea_t	area	= {0, 0, win_size.w, win_size.h};

	PtSetArg(&args_win[0], Pt_ARG_MIN_WIDTH, area.size.w, 0);
	PtSetArg(&args_win[1], Pt_ARG_MIN_HEIGHT, area.size.h, 0);
	PtSetArg(&args_win[3], Pt_ARG_MAX_HEIGHT, area.size.w, 0);
	PtSetArg(&args_win[4], Pt_ARG_MAX_WIDTH,  area.size.h, 0);
	PtSetArg(&args_win[5], Pt_ARG_WINDOW_TITLE, "Sokoban", 0);
	PtSetArg(&args_win[6], Pt_ARG_WINDOW_RENDER_FLAGS,
		Ph_WM_RENDER_ASAPP |
		Ph_WM_RENDER_CLOSE |
		Ph_WM_RENDER_TITLE |
		Ph_WM_RENDER_MIN,
		Pt_TRUE);

	PtSetArg(&args_win[7], Pt_ARG_WINDOW_CURSOR_OVERRIDE, Pt_TRUE, 0);
	PtSetArg(&args_win[8], Pt_ARG_WINDOW_STATE, Ph_WM_STATE_ISFRONT, 0);
	PtSetArg(&args_win[9], Pt_ARG_CURSOR_TYPE, Ph_CURSOR_NONE, 0);

	PtSetParentWidget(NULL);
	window = PtCreateWidget(PtWindow, NULL, 10, args_win);

	PtAddEventHandler(window, Ph_EV_KEY, &keyboard_callback, NULL);

	buf_draw		= new PhImage_t;
	buf_draw->type	= Pg_IMAGE_DIRECT_888;
	buf_draw->size	= area.size;
	buf_draw->image = new char[area.size.w * area.size.h * 3];

	PtSetArg(&args_lbl[0], Pt_ARG_LABEL_TYPE, Pt_IMAGE, 0 );
	PtSetArg(&args_lbl[1], Pt_ARG_AREA, &area, 0 );
	PtSetArg(&args_lbl[2], Pt_ARG_LABEL_DATA, buf_draw, sizeof(*buf_draw));
	PtSetArg(&args_lbl[3], Pt_ARG_MARGIN_HEIGHT, 0, 0);
	PtSetArg(&args_lbl[4], Pt_ARG_MARGIN_WIDTH, 0, 0);
	PtSetArg(&args_lbl[5], Pt_ARG_BORDER_WIDTH, 0, 0);

	label = PtCreateWidget(PtLabel, window, 6, args_lbl);

	PhPoint_t translation = { 0, 0 };
	mc = PmMemCreateMC(buf_draw, &win_size, &translation);

	PxMethods_t methods_brick;
	PxMethods_t methods_box;
	PxMethods_t methods_box_place;
	
	memset(&methods_brick, 0, sizeof(PxMethods_t));
	memset(&methods_box, 0, sizeof(PxMethods_t));
	memset(&methods_box_place, 0, sizeof(PxMethods_t));

	methods_brick.px_alloc			= img_memory_allocate;
	methods_brick.px_free			= img_memory_free;
	methods_brick.px_warning		= img_warning;
	methods_brick.px_error			= img_error;
	methods_brick.px_progress		= img_progress;
	methods_brick.flags				= PX_LOAD;

	methods_box.px_alloc			= img_memory_allocate;
	methods_box.px_free				= img_memory_free;
	methods_box.px_warning			= img_warning;
	methods_box.px_error			= img_error;
	methods_box.px_progress			= img_progress;
	methods_box.flags				= PX_LOAD;

	methods_box_place.px_alloc		= img_memory_allocate;
	methods_box_place.px_free		= img_memory_free;
	methods_box_place.px_warning	= img_warning;
	methods_box_place.px_error		= img_error;
	methods_box_place.px_progress	= img_progress;
	methods_box_place.flags			= PX_LOAD;

	soko_home[0] = String(String(getenv("HOME")) + "/.ptsoko/");
	soko_home[1] = String("/usr/ptsoko/");
	soko_home[2] = String("./");

	String textures_path[3];

	textures_path[0] = texture_find("box.bmp");
	textures_path[1] = texture_find("box_place.bmp");
	textures_path[2] = texture_find("brick.bmp");

	if (textures_path[0].length() == 0 || textures_path[1].length() == 0 || textures_path[2].length() == 0) {
		throw Game_ex("Textures not found");
	}

	textures.box		= PxLoadImage(strdup((const char *)textures_path[0]), &methods_box);
	textures.box_place	= PxLoadImage(strdup((const char *)textures_path[1]), &methods_box_place);
	textures.brick		= PxLoadImage(strdup((const char *)textures_path[2]), &methods_brick);

	if (textures.box->size.w != GAME_BLOCK_SIZE) {
		throw Game_ex(Help::Sprintf("Texture width must be: %d", GAME_BLOCK_SIZE));
	}

	if (textures.box_place->size.w != GAME_BLOCK_SIZE) {
		throw Game_ex(Help::Sprintf("Texture width must be: %d", GAME_BLOCK_SIZE));
	}

	if (textures.brick->size.w != GAME_BLOCK_SIZE) {
		throw Game_ex(Help::Sprintf("Texture width must be: %d", GAME_BLOCK_SIZE));
	}

	if (textures.box->size.h != GAME_BLOCK_SIZE) {
		throw Game_ex(Help::Sprintf("Texture height must be: %d", GAME_BLOCK_SIZE));
	}

	if (textures.box_place->size.h != GAME_BLOCK_SIZE) {
		throw Game_ex(Help::Sprintf("Texture height must be %d", GAME_BLOCK_SIZE));
	}

	if (textures.brick->size.h != GAME_BLOCK_SIZE) {
		throw Game_ex(Help::Sprintf("Texture height must be %d", GAME_BLOCK_SIZE));
	}

	state				= STATE_INIT;
	level_current		= 0;

	status_font			= new char[strlen("pcterm14")];
	strcpy(status_font, "pcterm14");
	status_height		= get_string_height(status_font, "Status");

	PtRealizeWidget(window);
	app = PtDefaultAppContext();

}

Game::~Game() {
}

String Game::texture_find(String path) {
	String texture_path;

	for (size_t i = 0; i < 3; ++i) {
		texture_path = String(soko_home[i] + "textures/" + path);
		FILE *f = fopen((const char *) texture_path, "r");

		if (f == NULL) {
			printf("error: Couldn't open file \"%s\"\n", (const char *)texture_path);
			texture_path = String("");
			continue;
		} else {
			printf("Texture: %s OK\n",  (const char *)texture_path);
			fclose(f);
			break;
		}
	}

	return texture_path;
}

void Game::init() {
	DIR *dir = NULL;
	struct dirent *entry;
	String levels_path[3];
	String levels_dir;

	levels_path[0] = String(soko_home[0] + "levels/");
	levels_path[1] = String(soko_home[1] + "levels/");
	levels_path[2] = String(soko_home[2] + "levels/");

	for (size_t i = 0; i < 3; ++i) {
		printf("Searching levels at \"%s\"\n", (const char *)levels_path[i]);
		dir = opendir((const char *)levels_path[i]);

		if (!dir) {
			printf("Couldn't open levels directory\n");
		} else {
			levels_dir = levels_path[i];
			printf("OK\n");
			break;
		}
	}

	if (!dir) {
		throw Game_ex("Levels not found\n");
	}

	while ((entry = readdir(dir)) != NULL) {
		String fname = String(entry->d_name);

		int i = fname.index(".lvl", fname.length() - 4);

		if (i == -1) {
			continue;
		}

		printf("Loading %s: ", (const char *) fname);
		fname = levels_dir + fname;

		FILE *level_file = fopen((const char *) fname, "r");

		if (level_file == NULL) {
			printf("error: Couldn't open file \"%s\"\n", (const char *)fname);
			continue;
		}

		char line[1024];

		String			level;
		unsigned int 	boxes		= 0;
		unsigned int 	box_places	= 0;
		unsigned int	player		= 0;
		unsigned int 	width		= 0;
		unsigned int 	height 		= 0;

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

				if (width > blocks_w) {
					throw Game_ex(Help::Sprintf("Broken level file. Level width to high. Must be <= %d", blocks_w));
				}
			}

			width = 0;
			++height;

			if (height > blocks_h) {
				throw Game_ex(Help::Sprintf("Broken level file. Level height to high. Must be <= %d", blocks_h));
			}

			level += '\n';
		}

		fclose(level_file);

		if (player == 0) {
			throw Game_ex("Broken level file. Player = 0");
		}

		if (boxes == 0) {
			throw Game_ex("Broken level file. Box = 0");
		}

		if (box_places == 0) {
			throw Game_ex("Broken level file. Box_place = 0");
		}

		if (boxes != box_places) {
			throw Game_ex("Broken level file. Box != Box_place");
		}

		if (player > 1) {
			throw Game_ex("Broken level file. Player > 1");
		}

		level = fname + ";" + level;

		levels.insert(level);

		printf("OK\n");
	}

	if (levels.entries() == 0) {
		throw Game_ex("Levels not found");
	}

	state = STATE_SPLASH;
	draw();
}

void Game::run() {
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
	objects_pos_t *positions = new objects_pos_t;

	if (!player_only) {
		for (size_t i = 0; i < boxes.entries(); ++i) {
			Object *obj			= boxes[i];
			object_pos_t *pos 	= new object_pos_t;

			*pos 				= obj->get_pos();

			positions->insert(pos);
		}		
	}

	Object *obj = objects[objects.entries() - 1];

	object_pos_t *pos 	= new object_pos_t;
	*pos 				= obj->get_pos();
	
	positions->insert(pos);

	story.insert(positions);
}

void Game::story_back() {
	if (story.entries() == 0) {
		return;
	}

	objects_pos_t *last_move = story[story.entries() - 1];

	//Restore Box positions
	if (last_move->entries() > 1) {
		for (size_t i = 0; i < last_move->entries() - 1; ++i) {
			object_pos_t *pos = (*last_move)[i];
			Object *obj = boxes[i];

			obj->set_pos(pos->x, pos->y);
		}
	}

	//Restore Player position
	object_pos_t *pos = (*last_move)[last_move->entries() - 1];
	Object *obj = objects[objects.entries() - 1];

	obj->set_pos(pos->x, pos->y);

	delete last_move;
	story.removeLast();
	moves--;
}

void Game::story_clear() {
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

void Game::player_move(Player *player, direction_t dir) {
	object_pos_t	player_pos_next	= player->move_next(dir);
	object_pos_t	player_pos		= player->get_pos();
	Box *			box				= NULL;

	bool is_move_correct = true;

	for (size_t i = 0; i < objects.entries(); ++i) {
		Object *		object	= objects[i];
		object_pos_t	obj_pos = object->get_pos();

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
		if (box != NULL) {
			object_pos_t box_pos_next = box->move_calc(dir);

			for (size_t i = 0; i < objects.entries(); ++i) {

				Object *		object	= objects[i];
				object_pos_t	obj_pos = object->get_pos();

				switch(object->get_type()) {
					case OBJECT_BOX: {
						if (((Box *) object) == box) {
							continue;
						}
					}
					case OBJECT_BRICK:{
						if (check(box_pos_next, obj_pos)) {
							return;
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

		player->move(dir);
		this->moves++;

		for (size_t i = 0; i < boxes.entries(); ++i) {
			Box *box = (Box *) boxes[i];

			bool find = false;
			for (size_t j = 0; j < box_places.entries(); ++j) {
				Box_place *box_place = (Box_place *) box_places[j];

				if (check(box->get_pos(), box_place->get_pos())) {
					find = true;
					break;
				}
			}

			if (find == false) {
				return;
			}
		}

		level_next();
	}
}


void Game::key_process(unsigned int key) {
	switch(state) {
		case STATE_SPLASH: {
			if (key == Pk_s && state == STATE_SPLASH) {
				level_load(level_current);
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
				return;
			}
			
			switch(key) {
				case Pk_Up: {
					player_move(player, DIRECTION_UP);
					break;
				}
				case Pk_Down: {
					player_move(player, DIRECTION_DOWN);
					break;
				}
				case Pk_Left: {
					player_move(player, DIRECTION_LEFT);
					break;
				}
				case Pk_Right: {
					player_move(player, DIRECTION_RIGHT);
					break;
				}
				case Pk_BackSpace: {
					story_back();
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
			}
			break;
		}
	}
	draw();
}

void Game::draw_string(unsigned int x, unsigned int y, char *str, char *font, unsigned int color) {
	PhPoint_t p;
	p.x = x;
	p.y = y;

	PgSetFont(font);
	PgSetTextColor(color);
	PgDrawText(str, strlen(str), &p, 0);
}

unsigned int Game::get_string_width(char *font, char *str) {
	PhRect_t rect;

	PfExtentText(&rect, NULL, font, str, strlen(str));

	return rect.lr.x - rect.ul.x + 1;
}

unsigned int Game::get_string_height(char *font, char *str) {
	PhRect_t rect;

	PfExtentText(&rect, NULL, font, str, strlen(str));

	return rect.lr.y - rect.ul.y + 1;
}

void Game::draw() {
	PmMemStart(mc);

	PgSetFillColor(0x0A0A0A);
	PgDrawIRect(0, 0, win_size.w, win_size.h, Pg_DRAW_FILL );

	switch(state) {
		case STATE_SPLASH: {
			char			str[100];
			unsigned int	h;
			int				s;
			unsigned int	x;
			unsigned int 	y;

			sprintf(str, "Sokoban for QNX4.25/Photon v%s", GAME_VERSION);
			h = get_string_height("pcterm20", str);

			x = h / 2;
			y = 0;
			s = 1;

			draw_string(x, y + h + s, str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "Control: ");
			draw_string(x, y + (h * 2) + s, str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "N - Next level");
			draw_string(x, y + (h * 3) + s, str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "P - Previous level");
			draw_string(x, y + (h * 4) + s, str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "R - Restart level");
			draw_string(x, y + (h * 5) + s, str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "Backspace - Undo");
			draw_string(x, y + (h * 6) + s, str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "Press S to start");
			draw_string((win_size.w / 2) - (get_string_width("pcterm20", str) / 2), 
				(win_size.h / 2), 
				str, "pcterm20", 0xF0F0F0);

			_bprintf(str, 100, "(C) %s 2019", GAME_AUTHOR);
			draw_string(win_size.w - get_string_width("pcterm20", str) - (h / 2), win_size.h - (h / 2), str, "pcterm20", 0xF0F0F0);

			break;
		}
		case STATE_INIT: {
			break;
		}
		case STATE_LOADING: {
			break;
		}
		case STATE_END: {
			char str[8];

			_bprintf(str, 8, "The END");

			draw_string((win_size.w / 2) - (get_string_width("pcterm20", str) / 2), (win_size.h / 2), str, "pcterm20", 0xF0F0F0);
			break;
		}
		case STATE_GAME: {
			PhPoint_t p;
			
			char status_level[50];
			char status_moves[12];

			_bprintf(status_level, 50, "Level: %s", (const char *) level_name());
			_bprintf(status_moves, 12, "Moves: %4d", moves);

			p.x = (status_height / 2) + 1;
			p.y = win_size.h - (status_height / 2);

			draw_string(p.x, p.y, status_level, status_font, 0xF0F0F0);

			p.x = win_size.w - get_string_width(status_font, status_moves) - ((status_height / 2) + 1);

			draw_string(p.x, p.y, status_moves, status_font, 0xF0F0F0);

			for (size_t i = 0; i < objects.entries(); ++i) {
				Object *object = objects[i];
				object->draw();
			}
			break;
		}
	}

	PmMemFlush(mc, buf_draw);
	PmMemStop(mc);

	PtArg_t args[1];
	PtSetArg( &args[0], Pt_ARG_LABEL_DATA, buf_draw, sizeof(*buf_draw));
	PtSetResources(label, 1, args);
}

static int Game::keyboard_callback(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) {
	if (info->event->type == Ph_EV_KEY) {
		PhKeyEvent_t *	ke		= (PhKeyEvent_t *) PhGetData(info->event);
		Game *			game	= &Game::get_instance();

		if (PkIsFirstDown(ke->key_flags)) {
			game->key_process(ke->key_cap);
		} else if (PkIsReleased(ke->key_flags)) {
			//game->key_process(ke->key_cap, false, true);
		} else {
			game->key_process(ke->key_cap);
		}
	}
	return Pt_CONTINUE;
}

void Game::set_state(game_state_t state) {
	this->state = state;
	draw();
}

String Game::level_name() {
	int name_index = levels[level_current].index(";");

	return String(levels[level_current], 0, name_index);
}

void Game::level_restart() {
	set_state(STATE_INIT);
	level_unload();
	level_load(level_current);
}

void Game::level_next() {
	set_state(STATE_INIT);
	level_unload();

	level_current++;

	if (level_current < levels.entries()) {
		level_load(level_current);
	} else {
		set_state(STATE_END);
	}
}

void Game::level_prev() {
	set_state(STATE_INIT);
	level_unload();

	if (level_current >= 1) {
		level_load(--level_current);
	} else {
		set_state(STATE_SPLASH);
	}
}

void Game::level_load(size_t index) {
	set_state(STATE_LOADING);

	int name_index = levels[level_current].index(";");

	String level = String(levels[level_current], name_index + 1);

	size_t 			i;
	unsigned int	x			= 0;
	unsigned int	y			= 0;
	
	unsigned int	x_max		= 0;
	unsigned int	y_max		= 0;
	
	unsigned int 	x_offset	= 0;
	unsigned int	y_offset	= 0;

	unsigned int	step		= GAME_BLOCK_SIZE;

	Player *		player		= NULL;

	for (i = 0; i < level.length(); ++i) {
		char type = level[i];

		switch(type) {
			case LVL_BTYPE_BRICK: {
				Brick *brick = new Brick(x, y, step - 1, step - 1, textures.brick);

				objects.insert((Object *)brick);
				break;
			}
			case LVL_BTYPE_BOX: {
				Box *box = new Box(x, y, step - 1, step - 1, textures.box);

				boxes.insert((Object *)box);
				break;
			}
			case LVL_BTYPE_BOX_PLACE: {
				Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures.box_place);

				box_places.insert((Object *)box_place);
				break;
			}
			case LVL_BTYPE_BOXWPLACE: {
				Box *box = new Box(x, y, step - 1, step - 1, textures.box);
				Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures.box_place);

				boxes.insert((Object *)box);
				box_places.insert((Object *)box_place);
				break;
			}
			case LVL_BTYPE_PLAYER: {
				player = new Player(x, y, step - 1, step - 1);
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
	y_offset = (win_size.h - y_max - status_height - (status_height / 2)) / 2;

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
	}

	moves = 0;
	set_state(STATE_GAME);
}

void Game::level_unload() {
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
		}

		objects.removeFirst();
	}

	while (boxes.entries() != 0) {
		boxes.removeFirst();
	}

	while (box_places.entries() != 0) {
		box_places.removeFirst();
	}
}

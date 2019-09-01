#include "game.h"
#include <String.h>
#include <stdio.h>

Game::Game() {
	PhChannelParms_t parms		= {0, 0, Ph_DYNAMIC_BUFFER};
	char *		displayname	= NULL;

	if (!PhAttach(displayname, &parms)) {
		if (displayname)
			printf("Could not attach to Photon manager [%s]", displayname);
		else if ((displayname = getenv("PHOTON")))
			printf("Could not attach to Photon manager (PHOTON=[%s])", displayname);
		else
			printf("Could not attach to Photon manager [/dev/photon]");
	}

	PtInit(NULL);

	win_size.w = 620;
	win_size.h = 508;

	PtArg_t		args_win[10];
	PtArg_t		args_raw[2];
	PtArg_t 	args_dbc[4];
	PhPoint_t	dbc_pos = {0, 0};
	PhDim_t		dbc_dim = {win_size.w, win_size.h};
	PhArea_t	dbc_area = {0, 0, win_size.w, win_size.h};

	PtSetArg(&args_win[0], Pt_ARG_MIN_WIDTH, dbc_dim.w, 0);
	PtSetArg(&args_win[1], Pt_ARG_MIN_HEIGHT, dbc_dim.h, 0);
	PtSetArg(&args_win[3], Pt_ARG_MAX_HEIGHT, dbc_dim.w, 0);
	PtSetArg(&args_win[4], Pt_ARG_MAX_WIDTH,  dbc_dim.h, 0);
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

	PtSetArg(&args_dbc[0], Pt_ARG_AREA, &dbc_area, 0);
	PtSetArg(&args_dbc[1], Pt_ARG_DB_IMAGE_TYPE, Pg_IMAGE_PALETTE_BYTE, 0);
	PtSetArg(&args_dbc[2], Pt_ARG_DB_MEMORY_CONTEXT_TYPE, Pm_PHS_CONTEXT, 0);

	dbc = PtCreateWidget(PtDBContainer, window, 3, args_dbc);

	PtSetArg(&args_raw[0], Pt_ARG_RAW_DRAW_F, &draw, 1);
	PtSetArg(&args_raw[1], Pt_ARG_DIM, &dbc_dim, 0);
	
	raw = PtCreateWidget(PtRaw, dbc, 2, args_raw);

	PtRealizeWidget(window);
	app = PtDefaultAppContext();

	textures.box		= PxLoadImage("textures/box.bmp", NULL);//PxLoadImage("textures/box.bmp", NULL);
	textures.box_place	= PxLoadImage("textures/box_place.bmp", NULL);
	textures.brick		= PxLoadImage("textures/brick.bmp", NULL);

	state		= STATE_INIT;
	level_current	= 0;
}

void Game::init() {
	DIR *dir;
	struct dirent *entry;

	dir = opendir("levels/");

	if (!dir) {
		throw Game_ex("Couldn't open levels directory");
	}

	while ((entry = readdir(dir)) != NULL) {
		String fname = String(entry->d_name);

		int i = fname.index(".lvl", fname.length() - 4);

		if (i == -1) {
			continue;
		}

		levels.insert(fname);
	}

	if (levels.entries() == 0) {
		throw Game_ex("Levels not found");
	}

	level_load(level_current);
}

PhDim_t Game::get_winsize() {
	return win_size;
}

void Game::photon_event() {
	union {
		void *raw;
		PhKeyEvent_t     *key_ev;
	} ph_ev;

	app->event->processing_flags = 0;
	ph_ev.raw = PhGetData(app->event);

	switch(app->event->type) {
		case Ph_EV_WM: {
			//printf("window event!\n");
			break;
		}
		case Ph_EV_KEY: {
			if (PkIsFirstDown(ph_ev.key_ev->key_flags)) {
				key_process(ph_ev.key_ev->key_cap, true, false);
      			} else if (PkIsReleased(ph_ev.key_ev->key_flags)) {
				key_process(ph_ev.key_ev->key_cap, false, true);      				
      			} else {
      				key_process(ph_ev.key_ev->key_cap, false, false);  
      			}
			PtDamageWidget(raw);      			
      			break;
		}
	}
	PtEventHandler(app->event);
}

void Game::photon_process() {
	register int ret;

	while((ret = PhEventPeek(app->event, app->event_size)))
	{
		switch(ret)
		{
			case Ph_EVENT_MSG:
				photon_event();
			break;
			case Ph_RESIZE_MSG:
				if (PtResizeEventMsg(app, PhGetMsgSize(app->event)) == -1)
					printf("Can not reallocate event buffer\n");
			break;
			case -1:
				printf("Receiving Photon event\n");
			break;
		}
	}

}

void Game::tick() {
	photon_process();
	//PtDamageWidget(raw);
}

bool check(object_post_t r1, object_post_t r2) {
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

void Game::player_move(Player *player, direction_t dir) {
	object_post_t	player_pos_next	= player->move_next(dir);
	object_post_t	player_pos	= player->get_pos();
	Box *		box		= NULL;

	bool is_move_correct = true;

	for (size_t i = 0; i < objects.entries(); ++i) {
		Object *	object	= objects[i];
		object_post_t	obj_pos = object->get_pos();

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
			object_post_t box_pos_next = box->move_calc(dir, player_pos_next.x, player_pos_next.y);

			for (size_t i = 0; i < objects.entries(); ++i) {

				Object *	object	= objects[i];
				object_post_t	obj_pos = object->get_pos();

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
			box->move(dir, player_pos_next.x, player_pos_next.y);
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


void Game::key_process(unsigned int key, bool press, bool release) {
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
	
	if (release) {
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
			printf("Pk_BackSpace\n");
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
}

static void Game::draw(PtWidget_t *widget, PhTile_t *damage) {
	Game *game = &Game::get_instance();

	PhDim_t win_size = game->get_winsize();

	switch(game->get_state()) {
		case STATE_INIT: {
			PgSetFillColor(0x0A0A0A);
			PgDrawIRect(0, 0, win_size.w, win_size.h, Pg_DRAW_FILL );
			break;
		}
		case STATE_LOADING: {
			PgSetFillColor(0x0A0A0A);
			PgDrawIRect(0, 0, win_size.w, win_size.h, Pg_DRAW_FILL );
			break;
		}
		case STATE_GAME: {
			PgSetFillColor(0x0A0A0A);
			PgDrawIRect(0, 0, win_size.w, win_size.h, Pg_DRAW_FILL );

			PhPoint_t p;
			
			char status_str[1024];
			sprintf(status_str, "Level: %s   Moves: %d", (const char *) game->level_name(), game->moves);

			p.x = 10;
			p.y = win_size.h - 8;

			PgSetFont("pcterm14");
			PgSetTextColor(0xF0F0F0);
			PgDrawText(status_str, strlen(status_str), &p, 0);

			objects_t *objects = game->get_objects();

			for (size_t i = 0; i < objects->entries(); ++i) {
				Object *object = (*objects)[i];

				object->draw();
			}
		}
	}
}

objects_t * Game::get_objects() {
	return &this->objects;
}


void Game::set_state(game_state_t state) {
	this->state = state;
	PtDamageWidget(raw);
}

game_state_t Game::get_state() {
	return state;
}

String Game::level_name() {
	return levels[level_current];
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
	if (level_current >= 1) {
		set_state(STATE_INIT);
		level_unload();
		level_load(--level_current);
	}
}

void Game::level_load(size_t index) {
	set_state(STATE_LOADING);

	String fname = "levels/" + levels[index];

	FILE *level = fopen((const char *) fname, "r");

	if (level == NULL) {
		throw Game_ex("Couldn't load file \"" + fname + "\"");
	}

	char line[1024];

	unsigned int	x	= 0;
	unsigned int	y	= 0;
	
	unsigned int	x_max	= 0;
	unsigned int	y_max	= 0;
	
	unsigned int 	x_offset = 0;
	unsigned int	y_offset = 0;

	unsigned int	step	= 20;

	Player *	player	= NULL;

	while (fgets(line, sizeof(line), level) != NULL) {
		line[strlen(line) - 1] = '\0';

		for (int i = 0; i < strlen(line); ++i) {
			char type = line[i];

			switch(type) {
				case 'X':
				case '#': {
					Brick *brick = new Brick(x, y, step - 1, step - 1, textures.brick);

					objects.insert((Object *)brick);
					break;
				}
				case '*': {
					Box *box = new Box(x, y, step - 1, step - 1, textures.box);

					boxes.insert((Object *)box);
					break;
				}
				case '.': {
					Box_place *box_place = new Box_place(x, y, step - 1, step - 1, textures.box_place);

					box_places.insert((Object *)box_place);
					break;
				}
				case '@': {
					player = new Player(x, y, step - 1, step - 1);
					break;
				}
			}
			x += step;
			if (x_max < x) {
				x_max = x;
			}
		}

		y += step;

		if (y_max < y) {
			y_max = y;
		}

		x = 0;
	}

	fclose(level);

	x_offset = (win_size.w - x_max) / 2;
	y_offset = (win_size.h - y_max - 8 - 14) / 2;

	for (size_t i = 0; i < box_places.entries(); ++i) {
		objects.insert(box_places[i]);
	}

	for (i = 0; i < boxes.entries(); ++i) {
		objects.insert(boxes[i]);
	}

	objects.insert((Object *)player);

	for (i = 0; i < objects.entries(); ++i) {
		Object *obj = objects[i];

		object_post_t obj_pos = obj->get_pos();
		obj_pos.x += x_offset;
		obj_pos.y += y_offset;
		obj->set_pos(obj_pos.x, obj_pos.y);
	}

	moves = 0;
	set_state(STATE_GAME);
}

void Game::level_unload() {
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
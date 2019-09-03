#include "game.h"
#include <String.h>
#include <stdio.h>
#include <time.h>

static void *img_memory_allocate(long nbytes, int type) {
	if( type == PX_IMAGE ) {
		return( PgShmemCreate( nbytes, NULL ) );
	} else {
		return( calloc( 1, nbytes ) );
	}
}

static void *img_memory_free(void *memory, int type) {
	if (type == PX_IMAGE) {
		PgShmemDestroy( memory );
	} else {
		free( memory );
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
	printf("Texture load status:  %d.%d percent\n", percent >> 16, percent & 0xffff);
	return NULL;
}

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

	PgSetDrawBufferSize(0xFFFF);

	win_size.w = 620;
	win_size.h = 508;

	PtArg_t		args_win[10];
	PtArg_t		args_lbl[6];

	PhPoint_t	dbc_pos = {0, 0};
	PhDim_t		dim = {win_size.w, win_size.h};
	PhArea_t	area = {0, 0, win_size.w, win_size.h};

	PtSetArg(&args_win[0], Pt_ARG_MIN_WIDTH, dim.w, 0);
	PtSetArg(&args_win[1], Pt_ARG_MIN_HEIGHT, dim.h, 0);
	PtSetArg(&args_win[3], Pt_ARG_MAX_HEIGHT, dim.w, 0);
	PtSetArg(&args_win[4], Pt_ARG_MAX_WIDTH,  dim.h, 0);
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

	buf_draw	= new PhImage_t;
	buf_draw->type	= Pg_IMAGE_PALETTE_BYTE; // 3 bytes per pixel 
	buf_draw->size	= dim;
	buf_draw->image = (char *) PgShmemCreate(dim.w * dim.h * 3, NULL);

	PtSetArg(&args_lbl[0], Pt_ARG_LABEL_TYPE, Pt_IMAGE, 0 );
	PtSetArg(&args_lbl[1], Pt_ARG_AREA, &area, 0 );
	PtSetArg(&args_lbl[2], Pt_ARG_LABEL_DATA, buf_draw, sizeof(*buf_draw));
	PtSetArg(&args_lbl[3], Pt_ARG_MARGIN_HEIGHT, 0, 0);
	PtSetArg(&args_lbl[4], Pt_ARG_MARGIN_WIDTH, 0, 0);
	PtSetArg(&args_lbl[5], Pt_ARG_BORDER_WIDTH, 0, 0);

	label = PtCreateWidget(PtLabel, window, 6, args_lbl);

	PhPoint_t translation = { 0, 0 }, center, radii;
	mc = PmMemCreateMC(buf_draw, &win_size, &translation);

	PxMethods_t methods_brick;
	PxMethods_t methods_box;
	PxMethods_t methods_box_place;
	
	memset(&methods_brick, 0, sizeof(PxMethods_t));
	memset(&methods_box, 0, sizeof(PxMethods_t));
	memset(&methods_box_place, 0, sizeof(PxMethods_t));

	methods_brick.px_alloc		= img_memory_allocate;
	methods_brick.px_free		= img_memory_free;
	methods_brick.px_warning	= img_warning;
	methods_brick.px_error		= img_error;
	methods_brick.px_progress	= img_progress;
	methods_brick.flags		= PX_LOAD;

	methods_box.px_alloc		= img_memory_allocate;
	methods_box.px_free		= img_memory_free;
	methods_box.px_warning		= img_warning;
	methods_box.px_error		= img_error;
	methods_box.px_progress		= img_progress;
	methods_box.flags		= PX_LOAD;

	methods_box_place.px_alloc	= img_memory_allocate;
	methods_box_place.px_free	= img_memory_free;
	methods_box_place.px_warning	= img_warning;
	methods_box_place.px_error	= img_error;
	methods_box_place.px_progress	= img_progress;
	methods_box_place.flags		= PX_LOAD;

	textures.box		= PxLoadImage("textures/box.bmp", &methods_box);//PxLoadImage("textures/box.bmp", NULL);
	textures.box_place	= PxLoadImage("textures/box_place.bmp", &methods_box_place);
	textures.brick		= PxLoadImage("textures/brick.bmp", &methods_brick);

	state			= STATE_INIT;
	level_current	= 0;

	PtRealizeWidget(window);
	app = PtDefaultAppContext();
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
	object_pos_t	player_pos	= player->get_pos();
	Box *		box		= NULL;

	bool is_move_correct = true;

	for (size_t i = 0; i < objects.entries(); ++i) {
		Object *	object	= objects[i];
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
			object_pos_t box_pos_next = box->move_calc(dir, player_pos_next.x, player_pos_next.y);

			for (size_t i = 0; i < objects.entries(); ++i) {

				Object *	object	= objects[i];
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
			box->move(dir, player_pos_next.x, player_pos_next.y);
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
	PmMemStart( mc );

	PgSetFillColor(0x0A0A0A);
	PgDrawIRect(0, 0, win_size.w, win_size.h, Pg_DRAW_FILL );

	switch(state) {
		case STATE_SPLASH: {
			char str[1024];
			unsigned int h;
			unsigned int s = 1;
			sprintf(str, "Sokoban for QNX4.25 v%.1f", GAME_VERSION);
			h = get_string_height("pcterm20", str);

			draw_string(10, h + s, str, "pcterm20", 0xF0F0F0);

			sprintf(str, "Control: ");
			draw_string(10, (h * 2) + s, str, "pcterm20", 0xF0F0F0);

			sprintf(str, "N - Next level");
			draw_string(10, (h * 3) + s, str, "pcterm20", 0xF0F0F0);

			sprintf(str, "P - Previous level");
			draw_string(10, (h * 4) + s, str, "pcterm20", 0xF0F0F0);

			sprintf(str, "R - Restart level");
			draw_string(10, (h * 5) + s, str, "pcterm20", 0xF0F0F0);

			sprintf(str, "Backspace - Undo");
			draw_string(10, (h * 6) + s, str, "pcterm20", 0xF0F0F0);

			sprintf(str, "Press S to start");
			draw_string((win_size.w / 2) - (get_string_width("pcterm20", str) / 2), 
						(win_size.h / 2), 
						str, "pcterm20", 0xF0F0F0);

			sprintf(str, "(c) %s 2019", GAME_AUTHOR);
			draw_string(win_size.w - get_string_width("pcterm20", str) - 10, win_size.h - (h / 2), str, "pcterm20", 0xF0F0F0);

			break;
		}
		case STATE_INIT: {
			break;
		}
		case STATE_LOADING: {
			break;
		}
		case STATE_GAME: {
			PhPoint_t p;
			
			char status_str[1024];
			sprintf(status_str, "Level: %s   Moves: %d", (const char *) level_name(), moves);

			p.x = 10;
			p.y = win_size.h - 8;

			PgSetFont("pcterm14");
			PgSetTextColor(0xF0F0F0);
			PgDrawText(status_str, strlen(status_str), &p, 0);

			for (size_t i = 0; i < objects.entries(); ++i) {
				Object *object = objects[i];
				object->draw();
			}
		}
	}

	PmMemFlush(mc, buf_draw);
	PmMemStop(mc);

	PtArg_t args[1];
	PtSetArg( &args[0], Pt_ARG_LABEL_DATA, buf_draw, sizeof(*buf_draw));
	PtSetResources( label, 1, args );
}

static int Game::keyboard_callback(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) {
	if (info->event->type == Ph_EV_KEY) {
		PhKeyEvent_t *	ke	= (PhKeyEvent_t *) PhGetData(info->event);
		Game *		game	= &Game::get_instance();

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
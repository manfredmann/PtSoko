#ifndef GAME_H
#define GAME_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>
#include <dirent.h>
#include <String.h>
#include <wcvector.h>

#include "object.h"
#include "brick.h"
#include "box.h"
#include "box_place.h"
#include "player.h"

#define PkIsReleased( f ) ((f & (Pk_KF_Key_Down|Pk_KF_Key_Repeat)) == 0)

typedef WCValSortedVector<String>	levels_t;
typedef WCValOrderedVector<Object *>	objects_t;

typedef enum {
	STATE_INIT,
	STATE_LOADING,
	STATE_GAME,
	STATE_WIN,
	STATE_END,
} game_state_t;

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


		void 		init();
		void		tick();
		void 		set_state(game_state_t state);
		game_state_t	get_state();
		PhDim_t		get_winsize();

		void 		level_next();
		void 		level_prev();
		void		level_restart();
		void		level_load(size_t index);
		void		level_unload();
		String		level_name();

		objects_t *	get_objects();

	private:
		Game();
		Game& operator=( Game& );

		PhDim_t		win_size;
		PtAppContext_t	app;
		PtWidget_t *	window;
		PtWidget_t * 	raw;
		PtWidget_t *	dbc;
		objects_t	objects;

		objects_t	boxes;
		objects_t	box_places;

		textures_t	textures;
		game_state_t	state;
		levels_t	levels;
		size_t		level_current;

		unsigned int	moves;

		void		photon_event();
		void		photon_process();
		void 		key_process(unsigned int key, bool press, bool release);
		void 		player_move(Player *player, direction_t dir);

		static void	draw(PtWidget_t *widget, PhTile_t *damage);

};

#endif
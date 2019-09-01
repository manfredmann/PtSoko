#ifndef PLAYER_H
#define PLAYER_H

#include "object.h"
#include <math.h>

class Player : public Object {
	public:
		Player(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

		void		tick();
		void		draw();
		object_type_t	get_type();
		void		set_pos(unsigned int x, unsigned int y);

		object_post_t 	move_next(direction_t dir);

		void 		move(direction_t dir);

		object_post_t	get_pos();

	private:
		object_post_t	pos_start;
		object_post_t 	pos;
};

#endif
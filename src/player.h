#ifndef PLAYER_H
#define PLAYER_H

#include "object.h"
#include <math.h>

class Player : public Object {
	public:
		Player(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

		void			draw();
		object_type_t	get_type();
		void			set_pos(unsigned int x, unsigned int y);

		object_pos_t 	move_next(direction_t dir);

		void 			move(direction_t dir);

		object_pos_t	get_pos();

	private:
		object_pos_t	pos_start;
		object_pos_t 	pos;
};

#endif
#ifndef BOX_H
#define BOX_H

#include "object.h"
#include <math.h>

class Box : public Object {
	public:
		Box(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture);

		void			draw();
		object_type_t	get_type();
		void			set_pos(unsigned int x, unsigned int y);
		
		object_pos_t	move_calc(direction_t dir, unsigned int x, unsigned int y);
		void 			move(direction_t dir, unsigned int x, unsigned int y);

		object_pos_t	get_pos();


	private:
		object_pos_t	pos_start;
		object_pos_t 	pos;
		PhImage_t *		texture;
};

#endif
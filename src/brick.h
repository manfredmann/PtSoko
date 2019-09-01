#ifndef BRICK_H
#define BRICK_H

#include "object.h"

class Brick : public Object {
	public:
		Brick(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture);
		void 		tick();
		void		draw();

		object_type_t	get_type();
		void		set_pos(unsigned int x, unsigned int y);

		object_post_t	get_pos();
		void 		set_color(unsigned int);

	private:
		object_post_t 	pos;
		PhImage_t *	texture;
		
		unsigned int	color;
};

#endif
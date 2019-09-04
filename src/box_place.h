#ifndef BOX_PLACE_H
#define BOX_PLACE_H

#include "object.h"

class Box_place : public Object {
	public:
		Box_place(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture);
		void			draw();

		object_type_t	get_type();
		void			set_pos(unsigned int x, unsigned int y);

		object_pos_t	get_pos();

	private:
		object_pos_t 	pos;
		PhImage_t *		texture;
};

#endif
#include "box.h"

Box::Box(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture) {
	this->pos.x = x;
	this->pos.y = y;
	this->pos.w = w;
	this->pos.h = h;

	this->texture = texture;
}

void Box::tick() {

}

void Box::draw() {
	PhRect_t  rect;

	rect.ul.x = pos.x;
	rect.ul.y = pos.y;
	rect.lr.x = pos.x + pos.w;
	rect.lr.y = pos.y + pos.h;

	//PgSetUserClip(&rect);

	if (texture == NULL) {
		PgSetFillColor(0xAA5500);
		PgDrawIRect(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, Pg_DRAW_FILL);
	} else {
		PhPoint_t    p = { pos.x, pos.y };
		PgDrawImagemx(texture->image, texture->type, &p, &texture->size, texture->bpl, 0 );
	}	
}

object_type_t Box::get_type() {
	return OBJECT_BOX;
}

void Box::set_pos(unsigned int x, unsigned int y) {
	this->pos.x = x;
	this->pos.y = y;
}

object_post_t Box::get_pos() {
	return pos;
}

object_post_t Box::move_calc(direction_t dir, unsigned int x, unsigned int y) {
	object_post_t pos_next = pos;

	switch(dir) {
		case DIRECTION_UP: {
			pos_next.y = y - pos.h - 1;
			break;
		}
		case DIRECTION_DOWN: {
			pos_next.y = y + pos.h + 1;
			break;
		}
		case DIRECTION_LEFT: {
			pos_next.x = x - pos.w - 1;
			break;
		}
		case DIRECTION_RIGHT: {
			pos_next.x = x + pos.w + 1;
			break;
		}
	}

	return pos_next;
}


void Box::move(direction_t dir, unsigned int x, unsigned int y) {
	pos = move_calc(dir, x, y);
}

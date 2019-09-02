#include "box_place.h"

Box_place::Box_place(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture) {
	this->pos.x = x;
	this->pos.y = y;
	this->pos.w = w;
	this->pos.h = h;
	this->texture = texture;
}

void Box_place::draw() {
	PhRect_t  rect;

	rect.ul.x = pos.x;
	rect.ul.y = pos.y;
	rect.lr.x = pos.x + pos.w;
	rect.lr.y = pos.y + pos.h;

	PgSetUserClip(&rect);

	if (texture == NULL) {
		PgSetFillColor(0xFFFFFF);
		PgDrawIRect(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, Pg_DRAW_FILL);		
	} else {
		PhPoint_t    p = { pos.x, pos.y };
//		PgDrawPhImagemx(&p, texture, NULL);
		PgDrawImagemx(texture->image, texture->type, &p, &texture->size, texture->bpl, 0 );

	}

	PtClipRemove();
}

object_type_t Box_place::get_type() {
	return OBJECT_BOXPLACE;
}

void Box_place::set_pos(unsigned int x, unsigned int y) {
	this->pos.x = x;
	this->pos.y = y;
}

object_post_t Box_place::get_pos() {
	return pos;
}

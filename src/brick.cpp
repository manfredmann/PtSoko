/*
* PtSoko - Sokoban for QNX4.25/Photon
* Copyright (C) 2019 Roman Serov <roman@serov.co>
*
* This file is part of Sokoban for QNX4.25/Photon.
* 
* Sokoban for QNX4.25/Photon is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Sokoban for QNX4.25/Photon is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Sokoban for QNX4.25/Photon. If not, see <http://www.gnu.org/licenses/>.
*/

#include "brick.h"
#include <stdio.h>

Brick::Brick(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture) {
	this->pos.x = x;
	this->pos.y = y;
	this->pos.w = w;
	this->pos.h = h;
	this->color = 0xAA0000;
	this->texture = texture;
}

void Brick::draw() {
	PhRect_t  rect;

	rect.ul.x = pos.x;
	rect.ul.y = pos.y;
	rect.lr.x = pos.x + pos.w;
	rect.lr.y = pos.y + pos.h;

	PgSetUserClip(&rect);

	if (texture == NULL) {
		PgSetFillColor(this->color);
		PgDrawIRect(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, Pg_DRAW_FILL);
	} else {
		PhPoint_t p = { pos.x, pos.y };
//		PgDrawPhImagemx(&p, texture, NULL);
		PgDrawImagemx(texture->image, texture->type, &p, &texture->size, texture->bpl, 0 );
	}

	PtClipRemove();
}

object_type_t Brick::get_type() {
	return OBJECT_BRICK;
}

void Brick::set_pos(unsigned int x, unsigned int y) {
	this->pos.x = x;
	this->pos.y = y;
}

object_pos_t Brick::get_pos() {
	return pos;
}


void Brick::set_color(unsigned int color) {
	this->color = color;
}
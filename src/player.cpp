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

#include "player.h"

Player::Player(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	this->pos.x = x;
	this->pos.y = y;
	this->pos.w = w;
	this->pos.h = h;
}

void Player::draw() {
	PhRect_t  rect;
	
	rect.ul.x = pos.x;
	rect.ul.y = pos.y;
	rect.lr.x = pos.x + pos.w;
	rect.lr.y = pos.y + pos.h;

	PgSetUserClip(&rect);

	PgSetFillColor(0x005500);
	PgDrawIRect(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, Pg_DRAW_FILL);

	PtClipRemove();
}

object_type_t Player::get_type() {
	return OBJECT_PLAYER;
}

void Player::set_pos(unsigned int x, unsigned int y) {
	this->pos.x = x;
	this->pos.y = y;
}

object_pos_t Player::get_pos() {
	return pos;
}

object_pos_t Player::move_next(direction_t dir) {
	object_pos_t pos_curr = pos;

	switch(dir) {
		case DIRECTION_UP: {
			pos_curr.y -= pos.h + 1;
			break;
		}
		case DIRECTION_DOWN: {
			pos_curr.y += pos.h + 1;
			break;
		}
		case DIRECTION_LEFT: {
			pos_curr.x -= pos.w + 1;
			break;
		}
		case DIRECTION_RIGHT: {
			pos_curr.x += pos.w + 1;
			break;
		}
	}

	return pos_curr;
}

void Player::move(direction_t dir) {
	pos = move_next(dir);
}

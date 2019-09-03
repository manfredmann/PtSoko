#include "player.h"

Player::Player(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	this->pos.x = x;
	this->pos.y = y;
	this->pos.w = w;
	this->pos.h = h;
}

void Player::tick() {

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
	object_pos_t offset = move_next(dir);
	pos = move_next(dir);
}

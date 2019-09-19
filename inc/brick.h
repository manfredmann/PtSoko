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

#ifndef BRICK_H
#define BRICK_H

#include "object.h"

class Brick : public Object {
	public:
		Brick(unsigned int x, unsigned int y, unsigned int w, unsigned int h, PhImage_t *texture);
		void			draw();

		object_type_t	get_type();
		void			set_pos(unsigned int x, unsigned int y);

		object_pos_t	get_pos();
		void 			set_color(unsigned int);

	private:
		object_pos_t 	pos;
		PhImage_t *		texture;
		unsigned int	color;
};

#endif
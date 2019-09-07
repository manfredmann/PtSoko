/*
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

#ifndef OBJECT_H
#define OBJECT_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>

#define PX_IMAGE_MODULES
#define PX_BMP_SUPPORT
#define PX_PNG_SUPPORT

#include <photon/PxImage.h>

typedef unsigned char bool;
#define true 1
#define false 0

typedef enum {
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT,
} direction_t;

typedef enum {
	OBJECT_BRICK,
	OBJECT_BOX,
	OBJECT_BOXPLACE,
	OBJECT_PLAYER,
} object_type_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
} object_pos_t;

class Object {
	public:
		Object() {

		}

		virtual void 			draw();
		virtual object_type_t	get_type();
		virtual void			set_pos(unsigned int x, unsigned int y);
		virtual object_pos_t	get_pos();

	private:

};

#endif

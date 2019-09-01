#ifndef OBJECT_H
#define OBJECT_H

#include <Ap.h>
#include <Ph.h>
#include <Pt.h>

#define PX_IMAGE_MODULES
#define PX_BMP_SUPPORT

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
	OBJECT_COLOR,
} object_type_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
} object_post_t;

class Object {
	public:
		Object() {

		}

		virtual void 		tick();
		virtual void 		draw();
		virtual object_type_t	get_type();
		virtual void		set_pos(unsigned int x, unsigned int y);
		virtual object_post_t	get_pos();

	private:

};

#endif

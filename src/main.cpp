#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <i86.h>
#include <time.h>

#include "game.h"

int main(int argc, char **argv) {
	try {
		Game *game = &Game::get_instance();

		game->init();
		game->run();
	} catch (Game_ex e) {
		printf("Error: %s\n", (const char *) e.what());
	}

	return 0;
}

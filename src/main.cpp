#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <i86.h>
#include <time.h>

#include "game.h"

int main(int argc, char **argv) {
	Game *game = &Game::get_instance();

	try {
		game->init();

		while(1) {
			struct timespec tstart={0,0}, tend={0,0};
			clock_gettime(CLOCK_REALTIME, &tstart);

			game->tick();

			clock_gettime(CLOCK_REALTIME, &tend);

			double diff = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
			diff = 10.0 - (diff * 1000.0);

			if (diff > 0) {
				delay(diff);
			}
		}

	} catch (Game_ex e) {
		printf("Error: %s\n", (const char *) e.what());
	}

	return 0;
}

/*
* PtSoko - Sokoban for QNX4.25/Photon
* Copyright (C) 2019-2020 Roman Serov <roman@serov.co>
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

#include "timer.h"

Timer::Timer(unsigned int sec, unsigned int nsec) {
    is_init = false;

    proxy = qnx_proxy_attach(0, NULL, 0, -1);

    if (proxy == -1) {
        err = String("Unable to attach proxy");

        return;
    }

    event.sigev_signo = -proxy;
    timer_id = timer_create(CLOCK_REALTIME, &event);

    if (timer_id == -1) {
        err = String("Unable to attach timer");

        return;
    }

    is_init = true;

    timer.it_value.tv_sec       = sec;
    timer.it_value.tv_nsec      = nsec;
    timer.it_interval.tv_sec    = sec;
    timer.it_interval.tv_nsec   = nsec;

    timer_settime(timer_id, 0, &timer, NULL);
}

Timer::~Timer() {
    timer_delete(timer_id);
    qnx_proxy_detach(proxy);
}

pid_t Timer::get_proxy() {
    if (!is_init) {
        throw TimerException(err);
    }

    return proxy;
}

timer_t Timer::get_timer_id() {
    if (!is_init) {
        throw TimerException(err);
    }

    return timer_id;
}

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

#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <signal.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <bool.h>
#include <String.h>
#include <stdarg.h>
#include <stdio.h>

#include "exception.h"

class TimerException : public BaseException {
    public:
        TimerException(String msg) : BaseException(msg) { } 
        TimerException(const char *fmt, ...) : BaseException(fmt, (va_start(args_, fmt), args_)) {
            va_end(args_);
        }

    private:
        va_list args_;
};

class Timer {
    public:
        Timer(unsigned int sec, unsigned int nsec);
        ~Timer();

        pid_t   get_proxy();
        timer_t get_timer_id();

    private:
        pid_t               proxy;
        timer_t             timer_id;
        bool                is_init;
        struct itimerspec   timer;
        struct sigevent     event;
        String              err;
};

#endif

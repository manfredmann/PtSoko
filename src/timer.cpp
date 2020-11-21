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

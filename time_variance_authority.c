#include "codexion.h"

unsigned long long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((unsigned long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void precise_sleep(unsigned long long duration_ms, t_simulation *sim)
{
    unsigned long long start;
    unsigned long long elapsed;

    start = get_time_ms();
    while (1)
    {
        if (is_finished(sim))
            break;

        elapsed = get_time_ms() - start;
        if (elapsed >= duration_ms)
            break;

        usleep(200);
    }
}


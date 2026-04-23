#include "codexion.h"

long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void log_action(t_coder *c, char *action)
{
    long timestamp = get_time_ms() - c->sim->start_time;
    printf("%ld %ld %s\n", timestamp, c->coder_id, action);
}

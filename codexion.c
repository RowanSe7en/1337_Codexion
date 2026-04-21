/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/20 23:26:45 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void *coder_routine(void *arg)
{
    t_coder *c = (t_coder *)arg;

    while (!c->sim->stop)
    {
        printf("%d is thinking\n", c->coder_id);
        // usleep(1000);
    }
    return NULL;
}

int main(int ac, char **av)
{
    if (ac != 9)
    {
        printf("Pass exactly these number of arguments, with the EXACTE ");
        printf("order, and DO NOT miss any:\n");
        printf("number_of_coders time_to_burnout time_to_compile");
        printf("time_to_debug time_to_refactor number_of_compiles_required");
        printf(" dongle_cooldown scheduler\n");
        return 1;
    }

    t_arguments data = parser(ac, av);

    if (data.valid == 0)
        return 1;

    int size = data.number_of_coders;

    t_coder *coders = malloc(sizeof(t_coder) * size);
    t_dongle *dongles = malloc(sizeof(t_dongle) * size);

    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i;
        dongles[i].is_available = 1;
        // dongles[i].cooldown_until = 0;

        pthread_mutex_init(&dongles[i].mtx, NULL);
    }

    t_simulation sim;

    sim.args = data;
    sim.coders = coders;
    sim.dongles = dongles;
    sim.stop = 0;
    sim.start_time = get_time_ms();

    pthread_mutex_init(&sim.log_mtx, NULL);

    for (int i = 0; i < size; i++)
    {
        coders[i].coder_id = i + 1;
        coders[i].compile_count = 0;
        coders[i].last_compile_time = 0;
        coders[i].sim = &sim;

        coders[i].first_dongle = &dongles[i];
        coders[i].second_dongle = &dongles[(i + 1) % size];
    }

    for (int i = 0; i < size; i++)
    {
        pthread_create(&coders[i].coder, NULL, coder_routine, &coders[i]);
    }

    
    return 0;
}

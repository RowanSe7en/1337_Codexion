/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/22 17:23:56 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void set_finished(t_simulation *sim)
{
    for (size_t i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_lock(&sim->coders[i].state_mtx);
        size_t count = sim->coders[i].compile_count;
        pthread_mutex_unlock(&sim->coders[i].state_mtx);

        if (count != sim->args.number_of_compiles_required)
            return;
    }

    pthread_mutex_lock(&sim->is_finished_mtx);
    sim->is_finished = 1;
    pthread_mutex_unlock(&sim->is_finished_mtx);
}

int is_finished(t_simulation *sim)
{
    int answer;

    pthread_mutex_lock(&sim->is_finished_mtx);
    answer = sim->is_finished;
    pthread_mutex_unlock(&sim->is_finished_mtx);

    return answer;
}

void *main_loop(void *arg)
{
    printf("gggggggggggggggggg\n");
    t_coder *c = (t_coder *)arg;

    while (!is_finished(c->sim))
    {
        log_action(c, "is compiling");
        c->compile_count++;
        c->last_compile_time = get_time_ms();
        usleep(c->sim->args.time_to_compile);
        set_finished(c->sim);
    }
    printf("eeeeeeeeeeeeeeeeee\n");
    return NULL;
}

int main(int ac, char **av)
{
    if (ac != 9)
    {
        printf("Pass exactly these number of arguments, with the EXACTE ");
        printf("order, and DO NOT miss any:\n");
        printf("number_of_coders time_to_burnout time_to_compile ");
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

    t_simulation sim;

    sim.args = data;
    sim.coders = coders;
    sim.dongles = dongles;
    sim.is_finished = 0;
    sim.start_time = get_time_ms();
    
    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i;
        dongles[i].is_available = 1;
        // dongles[i].cooldown_until = 0;

        pthread_mutex_init(&dongles[i].mtx, NULL);
    }

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
        printf("aaaaaaaaaaaaaa\n");
        pthread_create(&coders[i].coder, NULL, main_loop, &coders[i]);
        printf("bbbbbbbbbbbbbb\n");
    }

    for (int i = 0; i < size; i++)
    {
        printf("cccccccccccccc\n");
        pthread_join(coders[i].coder, NULL);
        printf("dddddddddddddd\n");
    }
    
    return 0;
}

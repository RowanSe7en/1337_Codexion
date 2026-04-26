/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/26 20:14:05 by brouane          ###   ########.fr       */
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

int get_ready(t_simulation *sim)
{
    pthread_mutex_lock(&sim->is_ready_mtx);
    int is_ready = sim->is_all_ready;
    pthread_mutex_unlock(&sim->is_ready_mtx);
    return is_ready;
}

void wait_all_threads_ready(t_simulation *sim)
{
    int i = 0;

    while (!get_ready(sim))
        i++;
}

void *main_loop(void *arg)
{
    t_simulation *sim = (t_simulation *)arg;
    wait_all_threads_ready(sim);

    // while (!is_finished(sim))
    // {
    //     log_action(c, "is compiling");
    //     c->compile_count++;
    //     c->last_compile_time = get_time_ms();
    //     usleep(c->sim->args.time_to_compile);
    //     set_finished(c->sim);
    // }
    // printf("eeeeeeeeeeeeeeeeee\n");
    return NULL;
}

void program_starter(t_simulation *sim)
{
    int num_of_coders = sim->args.number_of_coders;
    printf("%d", num_of_coders);
    if (num_of_coders == 0)
        return ;
    // else if (num_of_coders == 1)
    //     ;
    else
    {
        for (int i = 0; i < num_of_coders; i++)
        {
            pthread_t *coder_thr = &sim->coders[i].coder;
            t_coder *coder = &sim->coders[i];

            pthread_create(coder_thr, NULL, main_loop, sim);
        }
    }

    sim->is_all_ready = 1;
    sim->start_time = get_time_ms();

    for (int i = 0; i < num_of_coders; i++)
    {
        pthread_t *coder_thr = &sim->coders[i].coder;
        pthread_join(coder_thr, NULL);
    }
    
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
    sim.is_all_ready = 0;

    pthread_mutex_init(&sim.is_ready_mtx, NULL);
    pthread_mutex_init(&sim.log_mtx, NULL);
    
    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i;
        dongles[i].is_available = 1;
        // dongles[i].cooldown_until = 0;

        pthread_mutex_init(&dongles[i].mtx, NULL);
    }


    for (int i = 0; i < size; i++)
    {
        coders[i].coder_id = i + 1;
        coders[i].compile_count = 0;
        coders[i].last_compile_time = 0;
        coders[i].sim = &sim;

        if (coders[i].coder_id % 2 == 0)
        {
            coders[i].first_dongle = &dongles[i];
            coders[i].second_dongle = &dongles[(i + 1) % size];
        }
        else
        {
            coders[i].second_dongle = &dongles[(i + 1) % size];
            coders[i].first_dongle = &dongles[i];
        }
        
    }

    program_starter(&sim);

    


    
    return 0;
}

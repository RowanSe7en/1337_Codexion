/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/27 00:00:03 by brouane          ###   ########.fr       */
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
    sim->is_all_ready = 1;
    pthread_mutex_unlock(&sim->is_ready_mtx);
    return sim->is_all_ready;
}

void wait_all_threads_ready(t_simulation *sim)
{
    int i = 0;

    while (!get_ready(sim))
    {
        pthread_mutex_lock(&sim->log_mtx);
        // printf("%d\n", i);
        i++;
        pthread_mutex_unlock(&sim->log_mtx);
        usleep(100);
    }
}

static void compile(t_coder *coder)
{
    pthread_mutex_lock(&coder->first_dongle->mtx);
    log_action(coder, "has taken a dongle");
    pthread_mutex_unlock(&coder->second_dongle->mtx);
    log_action(coder, "has taken a dongle");



    
    pthread_mutex_unlock(&coder->first_dongle->mtx);
    pthread_mutex_unlock(&coder->second_dongle->mtx);
}

void *main_loop(void *arg)
{
    // printf("f\n");
    t_code_sim *code_sim = (t_code_sim *)arg;
    // wait_all_threads_ready(code_sim->sim);

    // size_t required = code_sim->sim->args.number_of_compiles_required;
    // while (!is_finished(code_sim->sim))
    // {
    //     if (code_sim->coder->compile_count == required)
    //         break;

    //     compile(code_sim->coder);
        
    //     // log_action(c, "is compiling");
    //     // c->compile_count++;
    //     // c->last_compile_time = get_time_ms();
    //     // usleep(c->sim->args.time_to_compile);
    //     // set_finished(c->sim);
    // }
    // free(code_sim);
    // return NULL;
}

void program_starter(t_simulation *sim)
{
    int num_of_coders = sim->args.number_of_coders;
    printf("%dlllllllllllllll\n", num_of_coders);
    if (num_of_coders == 0)
        return ;
    // else if (num_of_coders == 1)
    //     ;
    else
    {
        for (int i = 0; i < num_of_coders; i++)
        {
            pthread_t *coder_thr = &sim->coders[i].coder;
\
            // t_code_sim *code_sim;
            // code_sim->coder = coder_thr;
            // code_sim->sim = sim;


            t_code_sim *code_sim = malloc(sizeof(t_code_sim));
            code_sim->coder = &sim->coders[i];
            code_sim->sim = sim;


            // pthread_create(coder_thr, NULL, main_loop, code_sim);
        }
    }
    // printf("ee %p\n", sim->coders[0].coder);
    // printf("rr %d\n", sim->coders[0].coder_id);
    // printf("tt %d\n", sim->coders[0].sim->coders[0].coder_id);

    // pthread_mutex_lock(&sim->is_ready_mtx);
    sim->is_all_ready = 1;
    // pthread_mutex_unlock(&sim->is_ready_mtx);
    sim->start_time = get_time_ms();

    // for (int i = 0; i < num_of_coders; i++)
    // {
    //     pthread_join(&sim->coders[i].coder, NULL);
    // }
    
}

void t(t_simulation *sim)
{
    sim->args.number_of_coders = 666;
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
    pthread_mutex_init(&sim.is_finished_mtx, NULL);
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
        // coders[i].last_compile_time = 0;
        coders[i].sim = &sim;
        pthread_mutex_init(&coders[i].state_mtx, NULL);

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


    // t(&sim);


    program_starter(&sim);

    


    
    return 0;
}

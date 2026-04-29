/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 23:53:35 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

unsigned short set_finished(t_simulation *sim)
{
    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_lock(&sim->coders[i].state_mtx);
        unsigned long count = sim->coders[i].compile_count;
        pthread_mutex_unlock(&sim->coders[i].state_mtx);

        if (count != sim->args.number_of_compiles_required)
            return 0;
    }

    pthread_mutex_lock(&sim->is_finished_mtx);
    sim->is_finished = 1;
    pthread_mutex_unlock(&sim->is_finished_mtx);
    return sim->is_finished;
}

unsigned short is_finished(t_simulation *sim)
{
    return set_finished(sim);
}

unsigned short get_ready(t_simulation *sim)
{
    pthread_mutex_lock(&sim->is_ready_mtx);
    unsigned short answer = sim->is_all_ready;
    pthread_mutex_unlock(&sim->is_ready_mtx);
    return answer;
}

void sync_threads(t_simulation *sim)
{
    int i = 0;

    while (!get_ready(sim))
        i++;
}

void compile(t_code_sim *code_sim)
{
    pthread_mutex_lock(&code_sim->coder->first_dongle->mtx);
    log_action(code_sim, "has taken a first dongle");
    pthread_mutex_lock(&code_sim->coder->second_dongle->mtx);
    log_action(code_sim, "has taken a second dongle");
    
    log_action(code_sim, "is compiling");
    precise_sleep(code_sim->sim->args.time_to_compile, code_sim->sim);
    
    pthread_mutex_lock(&code_sim->coder->state_mtx);
    code_sim->coder->last_compile_time = get_time_ms();
    code_sim->coder->compile_count++;
    pthread_mutex_unlock(&code_sim->coder->state_mtx);
    
    pthread_mutex_unlock(&code_sim->coder->first_dongle->mtx);
    pthread_mutex_unlock(&code_sim->coder->second_dongle->mtx);
}

void debug(t_code_sim *code_sim)
{ 
    log_action(code_sim, "is debugging");
    precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);

}

void refactor(t_code_sim *code_sim)
{ 
    log_action(code_sim, "is refactoring");
    precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
}

void *main_loop(void *arg)
{
    t_code_sim *code_sim = (t_code_sim *)arg;

    sync_threads(code_sim->sim);

    unsigned long required = code_sim->sim->args.number_of_compiles_required;
    while (!is_finished(code_sim->sim))
    {
        if (code_sim->coder->compile_count == required)
            break;

        compile(code_sim);
        debug(code_sim);
        refactor(code_sim);

    }
    return NULL;
}

void program_starter(t_simulation *sim)
{
    printf("program_starter\n");

    int num_of_coders = sim->args.number_of_coders;
    t_code_sim *codes_sims = malloc_for_me(sizeof(t_code_sim) * num_of_coders);

    if (!codes_sims)
        return ;
    
    if (num_of_coders == 0)
        return ;
    // else if (num_of_coders == 1)
    //     ;
    else
    {
        for (int i = 0; i < num_of_coders; i++)
        {
            codes_sims[i].sim = sim;
            codes_sims[i].coder = &sim->coders[i];

            thread_create(&sim->coders[i].coder, main_loop, &codes_sims[i]);
        }
    }

    lock_mutex(&sim->is_ready_mtx, sim);
    sim->is_all_ready = 1;
    sim->start_time = get_time_ms();
    unlock_mutex(&sim->is_ready_mtx, sim);

    for (int i = 0; i < num_of_coders; i++)
        thread_join(&sim->coders[i].coder, sim);

    free(codes_sims);
}

int main(int ac, char **av)
{
    if (ac != 9)        
        return bye_bye();

    t_arguments data = parser(ac, av);

    if (data.valid == 0)
        return 1;

    int size = data.number_of_coders;

    t_coder *coders = malloc_for_me(sizeof(t_coder) * size);
    t_dongle *dongles = malloc_for_me(sizeof(t_dongle) * size);

    if (!coders || !dongles)
        return freedom(coders, dongles);
    
    t_simulation sim;

    sim.args = data;
    sim.coders = coders;
    sim.dongles = dongles;
    sim.is_finished = 0;
    sim.is_all_ready = 0;

    initiate_mutex(&sim.log_mtx, &sim);
    initiate_mutex(&sim.is_ready_mtx, &sim);
    initiate_mutex(&sim.is_finished_mtx, &sim);

    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i + 1;
        dongles[i].is_available = 1;
        // dongles[i].cooldown_until = 0;

        initiate_mutex(&dongles[i].mtx, &sim);
    }


    for (int i = 0; i < size; i++)
    {
        coders[i].coder_id = i + 1;
        coders[i].compile_count = 0;
        coders[i].last_compile_time = 0;
        coders[i].sim = &sim;
        initiate_mutex(&coders[i].state_mtx, &sim);

        if (coders[i].coder_id % 2 == 0)
        {
            coders[i].first_dongle = &dongles[i];
            coders[i].second_dongle = &dongles[(i + 1) % size];
        }
        else
        {
            coders[i].first_dongle = &dongles[(i + 1) % size];
            coders[i].second_dongle = &dongles[i];
        }
        
    }

    program_starter(&sim);
    destroy_them_all(&sim);
    freedom(coders, dongles);

    return 0;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/05/09 23:35:51 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

short set_finished(t_simulation *sim, short finish_it)
{
    if (!finish_it)
    {
        for (int i = 0; i < sim->args.number_of_coders; i++)
        {
            pthread_mutex_lock(&sim->coders[i].state_mtx);
            long long count = sim->coders[i].compile_count;
            pthread_mutex_unlock(&sim->coders[i].state_mtx);
            
            if (count != sim->args.number_of_compiles_required)
            return 0;
        }
    }

    pthread_mutex_lock(&sim->is_finished_mtx);
    sim->is_finished = 1;
    pthread_mutex_unlock(&sim->is_finished_mtx);
    return sim->is_finished;
}

short is_finished(t_simulation *sim)
{
    return set_finished(sim, 0);
}

short get_ready(t_simulation *sim)
{
    pthread_mutex_lock(&sim->is_ready_mtx);
    short answer = sim->is_all_ready;
    pthread_mutex_unlock(&sim->is_ready_mtx);
    return answer;
}

void sync_threads(t_simulation *sim)
{
    int i = 0;

    while (!get_ready(sim))
        i++;
}

int get_counter(t_code_sim *code_sim)
{
    int answer;
    pthread_mutex_lock(&code_sim->coder->first_dongle->scheduler.counter_mtx);
    answer = code_sim->coder->first_dongle->scheduler.counter;
    pthread_mutex_unlock(&code_sim->coder->first_dongle->scheduler.counter_mtx);
    return answer;
}

void get_order(t_code_sim *code_sim)
{
    pthread_mutex_lock(&code_sim->coder->first_dongle->scheduler.counter_mtx);

    int *counter;
    long long *order;

    counter = &code_sim->coder->first_dongle->scheduler.counter;
    order   = code_sim->coder->first_dongle->scheduler.order;

    order[*counter] = code_sim->coder->coder_id;
    (*counter)++;

    printf("[get_order] Coder %d assigned order %d on dongle %d\n",
       code_sim->coder->coder_id,
       *counter,
       code_sim->coder->first_dongle->dongle_id);
    
    pthread_mutex_unlock(&code_sim->coder->first_dongle->scheduler.counter_mtx);
}

void edf(t_code_sim *code_sim)
{
    pthread_mutex_lock(&code_sim->coder->first_dongle->scheduler.counter_mtx);

    int *counter;
    long long *order;

    counter = &code_sim->coder->first_dongle->scheduler.counter;
    order = code_sim->coder->first_dongle->scheduler.order;
    
    pthread_mutex_lock(&code_sim->coder->state_mtx);
    long long last_compile_time = code_sim->coder->last_compile_time;
    pthread_mutex_unlock(&code_sim->coder->state_mtx);
    
    long long time_to_burnout = ms_to_us(code_sim->sim->args.time_to_burnout);
    
    order[*counter] = last_compile_time + time_to_burnout;

    printf("last_compile_time=%lld | time_to_burnout=%lld | total=%lld\n",
       last_compile_time,
       time_to_burnout,
       order[*counter]);

    (*counter)++;

    int x = *counter - 1;

    printf("----------------------------------------------------------------------------EDF: coder_id=%d | time[%d]=%lld | dongle_id=%d | counter=%d\n",
       code_sim->coder->coder_id,
       x,
       order[x],
       code_sim->coder->first_dongle->dongle_id,
       *counter);

    pthread_mutex_unlock(&code_sim->coder->first_dongle->scheduler.counter_mtx);
}

void reset_order(t_scheduler *scheduler)
{
    pthread_mutex_unlock(&scheduler->counter_mtx);
    scheduler->order[0] = 0;
    scheduler->order[1] = 0;
    scheduler->counter = 0;
    pthread_mutex_unlock(&scheduler->counter_mtx);

}

void compile(t_code_sim *code_sim)
{
    if (strcmp(code_sim->sim->args.scheduler , "edf") == 0)
    {
        edf(code_sim);
        while (get_counter(code_sim) < 2)
            ;

        long long last_compile_time = code_sim->coder->last_compile_time;
        long long time_to_burnout = ms_to_us(code_sim->sim->args.time_to_burnout);
        long long edf_time = last_compile_time + time_to_burnout;
        long long first_time = code_sim->coder->first_dongle->scheduler.order[0];
        long long second_time = code_sim->coder->first_dongle->scheduler.order[1];

        if (edf_time == first_time && edf_time >= second_time)
            ;
        else if (edf_time == second_time && edf_time > first_time)
            ;
        else
            precise_sleep(10, code_sim->sim);
    }
    // else if (strcmp(code_sim->sim->args.scheduler , "fifo") == 0)
    // {
    //     get_order(code_sim);
    //     while (get_counter(code_sim) < 2)
    //         ;
    //     printf("00000000000000000000000000000000000000000000: %d\n", code_sim->coder->first_dongle->scheduler.order[0]);
    //     printf("11111111111111111111111111111111111111111111: %d\n", code_sim->coder->first_dongle->scheduler.order[1]);
    //     if (code_sim->coder->first_dongle->scheduler.order[0] != code_sim->coder->coder_id)
    //         precise_sleep(10, code_sim->sim);
    // }
    // else
    //     printf("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh\n");

    pthread_mutex_lock(&code_sim->coder->first_dongle->reset_mtx);
    if (code_sim->coder->first_dongle->can_reset)
    {
        reset_order(&code_sim->coder->first_dongle->scheduler);
        code_sim->coder->first_dongle->can_reset = 0;
    }
    else
        code_sim->coder->first_dongle->can_reset = 1;
    pthread_mutex_unlock(&code_sim->coder->first_dongle->reset_mtx);
    

    pthread_mutex_lock(&code_sim->coder->first_dongle->dongle_mtx);
    log_action(code_sim, "has taken a first dongle");
    pthread_mutex_lock(&code_sim->coder->second_dongle->dongle_mtx);
    log_action(code_sim, "has taken a second dongle");
    
    // log_action(code_sim, "is compiling");
    // if (!code_sim->coder->is_regestered)
    //     fifo(code_sim);
    precise_sleep(code_sim->sim->args.time_to_compile, code_sim->sim);
    
    pthread_mutex_lock(&code_sim->coder->state_mtx);
    code_sim->coder->last_compile_time = get_time_us();
    code_sim->coder->compile_count++;
    pthread_mutex_unlock(&code_sim->coder->state_mtx);
    
    // log_action(code_sim, "has droped a first dongle");
    pthread_mutex_unlock(&code_sim->coder->first_dongle->dongle_mtx);
    // log_action(code_sim, "has droped a second dongle");
    pthread_mutex_unlock(&code_sim->coder->second_dongle->dongle_mtx);
}

void debug(t_code_sim *code_sim)
{ 
    // log_action(code_sim, "is debugging");
    precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);

}

void refactor(t_code_sim *code_sim)
{ 
    // log_action(code_sim, "is refactoring");
    precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
}

void *main_loop(void *arg)
{
    t_code_sim *code_sim = (t_code_sim *)arg;

    sync_threads(code_sim->sim);

    long long required = code_sim->sim->args.number_of_compiles_required;

    while (!is_finished(code_sim->sim))
    {
        // i think this mutex is not neccessery
        pthread_mutex_lock(&code_sim->coder->state_mtx);
        long long compile_count = code_sim->coder->compile_count;
        pthread_mutex_unlock(&code_sim->coder->state_mtx);

        if (compile_count == required)
            break;
        
        compile(code_sim);
        debug(code_sim);
        refactor(code_sim);

    }
    return NULL;
}

long long x = 0;

short check_if_coder_burned_out(t_simulation *sim)
{

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_lock(&sim->coders[i].state_mtx);
        long long last_compile_time = sim->coders[i].last_compile_time;
        pthread_mutex_unlock(&sim->coders[i].state_mtx);

        long long now = get_time_us();
        long long time = now - last_compile_time;

        if (time >= ms_to_us(sim->args.time_to_burnout))
            return set_finished(sim, 1);
    }
    return 0;
}

void check_if_all_compiles_done(t_simulation *sim)
{
    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_lock(&sim->coders[i].state_mtx);
        long long compile_count = sim->coders[i].compile_count;
        pthread_mutex_unlock(&sim->coders[i].state_mtx);

        if (compile_count != sim->args.number_of_compiles_required)
            return;
    }
    pthread_mutex_lock(&sim->is_finished_mtx);
    sim->is_finished = 1;
    pthread_mutex_unlock(&sim->is_finished_mtx);
    printf("done\n");
}

void *the_watcher(void *arg)
{
    t_simulation *sim = (t_simulation *)arg;

    sync_threads(sim);

    while (!is_finished(sim))
    {
        short burn = check_if_coder_burned_out(sim);
        if (burn)
        {
            printf("burnt out\n");
            exit(0);
        }
        
        if (!is_finished(sim) && !burn)
            check_if_all_compiles_done(sim);
    }
    
    return NULL;
}

void program_starter(t_simulation *sim)
{
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
        pthread_create(&sim->watcher_thread, NULL, the_watcher, sim);
    }

    lock_mutex(&sim->is_ready_mtx, sim);
    sim->is_all_ready = 1;
    sim->start_time = get_time_us();
    unlock_mutex(&sim->is_ready_mtx, sim);

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        sim->coders[i].last_compile_time = get_time_us();
    }

    for (int i = 0; i < num_of_coders; i++)
        thread_join(&sim->coders[i].coder, sim);
    pthread_join(sim->watcher_thread, NULL);
    exit(0);

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

    t_coder *coders = malloc(sizeof(t_coder) * size);
    t_dongle *dongles = malloc(sizeof(t_dongle) * size);

    if (!coders || !dongles)
        return freedom(coders, dongles);
    
    t_simulation sim;

    sim.args = data;
    sim.coders = coders;
    sim.dongles = dongles;
    sim.is_finished = 0;
    sim.is_all_ready = 0;
    // sim.order_array = malloc_for_me(sizeof(t_scheduler) * size);

    initiate_mutex(&sim.log_mtx, &sim);
    initiate_mutex(&sim.is_ready_mtx, &sim);
    initiate_mutex(&sim.is_finished_mtx, &sim);

    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i + 1;
        dongles[i].is_available = 1;
        dongles[i].can_reset = 0;
        // sim.order_array[i].counter = 0;
        dongles[i].scheduler.counter = 0;
        
        initiate_mutex(&dongles[i].dongle_mtx, &sim);
        initiate_mutex(&dongles[i].reset_mtx, &sim);
        initiate_mutex(&dongles[i].scheduler.counter_mtx, &sim);
    }


    for (int i = 0; i < size; i++)
    {
        coders[i].coder_id = i + 1;
        coders[i].compile_count = 0;
        coders[i].last_compile_time = get_time_us();
        // coders[i].is_regestered = 0;
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

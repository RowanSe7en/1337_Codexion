/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/05/15 21:54:13 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long get_last_compile_time(t_coder *coder)
{
    pthread_mutex_lock(&coder->state_mtx);
    long long answer = coder->last_compile_time;
    pthread_mutex_unlock(&coder->state_mtx);
    return answer;
}

long long get_last_used_time(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->used_time_mtx);
    long long answer = dongle->last_used_time;
    pthread_mutex_unlock(&dongle->used_time_mtx);
    return answer;
}

void set_last_compile_time(t_coder *coder, long long now)
{
    pthread_mutex_lock(&coder->state_mtx);
    coder->last_compile_time = now;
    pthread_mutex_unlock(&coder->state_mtx);
}

void set_last_used_time(t_dongle *dongle, long long time)
{
    pthread_mutex_lock(&dongle->used_time_mtx);
    dongle->last_used_time = time;
    pthread_mutex_unlock(&dongle->used_time_mtx);
}

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

int get_counter(t_dongle *dongle)
{
    int answer;
    pthread_mutex_lock(&dongle->scheduler.counter_mtx);
    answer = dongle->scheduler.counter;
    pthread_mutex_unlock(&dongle->scheduler.counter_mtx);
    return answer;
}

int get_passed(t_dongle *dongle)
{
    int answer;
    pthread_mutex_lock(&dongle->passed_mtx);
    answer = dongle->coders_passed;
    pthread_mutex_unlock(&dongle->passed_mtx);
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

void reset_order(t_scheduler *scheduler)
{
    pthread_mutex_unlock(&scheduler->counter_mtx);
    scheduler->order[0] = 0;
    scheduler->order[1] = 0;
    scheduler->counter = 0;
    pthread_mutex_unlock(&scheduler->counter_mtx);

}

void reset_passed(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->passed_mtx);
    dongle->coders_passed = 0;
    pthread_mutex_unlock(&dongle->passed_mtx);

}

void edf_organiser(t_coder *coder, t_dongle *dongle, t_simulation *sim)
{
    pthread_mutex_lock(&dongle->scheduler.counter_mtx);

    int *counter;
    long long *order;

    counter = &dongle->scheduler.counter;
    order = dongle->scheduler.order;
    
    long long last_compile_time = get_last_compile_time(coder);
    long long time_to_burnout = ms_to_us(sim->args.time_to_burnout);
    
    order[*counter] = last_compile_time + time_to_burnout;
    long long timestamp = get_time_ms() - us_to_ms(sim->start_time);
    printf("at %lld coder=%d time_to_burnout=  %lld, dongle_id=%d 11111111111111\n",
        timestamp,
        coder->coder_id,
       get_time_ms() - us_to_ms(last_compile_time),
       dongle->dongle_id
    );

    (*counter)++;

    // int x = *counter - 1;

    // printf("----------------------------------------------------------------------------EDF: coder_id=%d | time[%d]=%lld | dongle_id=%d | counter=%d\n",
    //    coder->coder_id,
    //    x,
    //    order[x],
    //    dongle->dongle_id,
    //    *counter);

    pthread_mutex_unlock(&dongle->scheduler.counter_mtx);
}

void edf_scheduler(t_code_sim *code_sim, t_dongle *dongle, int s_phase)
{
    edf_organiser(code_sim->coder, dongle, code_sim->sim);

    while ((!s_phase && get_passed(dongle) == 2 && get_counter(dongle) < 2) || (s_phase && get_passed(dongle) == 2 && get_counter(dongle) < 2))
        ;

    long long last_compile_time = get_last_compile_time(code_sim->coder);
    long long time_to_burnout = ms_to_us(code_sim->sim->args.time_to_burnout);
    long long edf_time = last_compile_time + time_to_burnout;
    long long first_time = dongle->scheduler.order[0];
    long long second_time = dongle->scheduler.order[1];

    if (edf_time == first_time && edf_time >= second_time)
        ;
    else if (edf_time == second_time && edf_time > first_time)
        ;
    else
        precise_sleep(10, code_sim->sim);

    pthread_mutex_lock(&dongle->reset_mtx);
    if (dongle->can_reset)
    {
        reset_order(&dongle->scheduler);
        reset_passed(dongle);
        dongle->can_reset = 0;
    }
    else
        dongle->can_reset = 1;
    pthread_mutex_unlock(&dongle->reset_mtx);
}

void compile(t_code_sim *code_sim)
{
    
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

    log_action(code_sim->sim, code_sim->coder, "has tried to take the first dongle");

    long long now = get_time_us();
    printf("at %lld coder %d should be waiting1: %lld####\n",get_time_ms() - us_to_ms(code_sim->sim->start_time), code_sim->coder->coder_id, now - now);
    while (get_time_us() - get_last_used_time(code_sim->coder->first_dongle) < ms_to_us(code_sim->sim->args.dongle_cooldown))
        ;
    printf("at %lld coder %d should bedone waiting1: %lld@@@@\n",get_time_ms() - us_to_ms(code_sim->sim->start_time), code_sim->coder->coder_id, get_time_us() - now);

    if (code_sim->sim->is_edf)
        edf_scheduler(code_sim, code_sim->coder->first_dongle, 0);

    pthread_mutex_lock(&code_sim->coder->first_dongle->dongle_mtx);
    
    pthread_mutex_lock(&code_sim->coder->second_dongle->passed_mtx);
    code_sim->coder->second_dongle->coders_passed++;
    pthread_mutex_unlock(&code_sim->coder->second_dongle->passed_mtx);
    long long timestamp = get_time_ms() - us_to_ms(code_sim->sim->start_time);
    long long last_compile_time = get_last_compile_time(code_sim->coder);

    printf("at %lld coder=%d time_to_burnout=  %lld, dongle_id=%d 22222222222222\n",
        timestamp,
        code_sim->coder->coder_id,
       get_time_ms() - us_to_ms(last_compile_time),
       code_sim->coder->first_dongle->dongle_id
    );

    log_action(code_sim->sim, code_sim->coder, "has taken a first dongle");

    log_action(code_sim->sim, code_sim->coder, "has tried to take the second dongle");
    
    now = get_time_us();
    printf("at %lld coder %d should be waiting2: %lld####\n",get_time_ms() - us_to_ms(code_sim->sim->start_time), code_sim->coder->coder_id, now - now);
    while (get_time_us() - get_last_used_time(code_sim->coder->second_dongle) < ms_to_us(code_sim->sim->args.dongle_cooldown))
        ;
    printf("at %lld coder %d should bedone waiting2: %lld@@@@\n",get_time_ms() - us_to_ms(code_sim->sim->start_time), code_sim->coder->coder_id, get_time_us() - now);
    
    if (code_sim->sim->is_edf)
        edf_scheduler(code_sim, code_sim->coder->second_dongle, 1);
    pthread_mutex_lock(&code_sim->coder->second_dongle->dongle_mtx);

    
    timestamp = get_time_ms() - us_to_ms(code_sim->sim->start_time);
    last_compile_time = get_last_compile_time(code_sim->coder);

    printf("at %lld coder=%d time_to_burnout=  %lld, dongle_id=%d 3333333333333333\n",
        timestamp,
        code_sim->coder->coder_id,
       get_time_ms() - us_to_ms(last_compile_time),
       code_sim->coder->first_dongle->dongle_id
    );
    log_action(code_sim->sim, code_sim->coder, "has taken a second dongle");
    
    log_action(code_sim->sim, code_sim->coder, "is compiling");

    precise_sleep(code_sim->sim->args.time_to_compile, code_sim->sim);
    
    set_last_compile_time(code_sim->coder, get_time_us());
    set_last_used_time(code_sim->coder->first_dongle, get_last_compile_time(code_sim->coder));
    set_last_used_time(code_sim->coder->second_dongle, get_last_compile_time(code_sim->coder));
    
    log_action(code_sim->sim, code_sim->coder, "has droped a first dongle");
    pthread_mutex_unlock(&code_sim->coder->first_dongle->dongle_mtx);
    log_action(code_sim->sim, code_sim->coder, "has droped a second dongle");
    pthread_mutex_unlock(&code_sim->coder->second_dongle->dongle_mtx);
}

void debug(t_code_sim *code_sim)
{ 
    log_action(code_sim->sim, code_sim->coder, "is debugging");
    precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);

}

void refactor(t_code_sim *code_sim)
{ 
    log_action(code_sim->sim, code_sim->coder, "is refactoring");
    precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
    pthread_mutex_lock(&code_sim->coder->state_mtx);
    code_sim->coder->compile_count++;
    pthread_mutex_unlock(&code_sim->coder->state_mtx);
    log_action(code_sim->sim, code_sim->coder, "is done refactoring");
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

short check_if_coder_burned_out(t_simulation *sim)
{

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        long long last_compile_time = get_last_compile_time(&sim->coders[i]);
        long long now = get_time_us();

        if (now - last_compile_time >= ms_to_us(sim->args.time_to_burnout))
        {
            log_action(sim, &sim->coders[i], "burned out");
            set_finished(sim, 1);
            exit(0);
        }
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
    printf("%lld done\n", get_time_ms());
    sim->is_finished = 1;
    pthread_mutex_unlock(&sim->is_finished_mtx);
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

    for (int i = 0; i < num_of_coders; i++)
        thread_join(&sim->coders[i].coder, sim);
    pthread_join(sim->watcher_thread, NULL);

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
    sim.is_edf = 0;
    if (strcmp(sim.args.scheduler , "edf") == 0)
        sim.is_edf = 1;

    initiate_mutex(&sim.log_mtx, &sim);
    initiate_mutex(&sim.is_ready_mtx, &sim);
    initiate_mutex(&sim.is_finished_mtx, &sim);

    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i + 1;
        dongles[i].is_available = 1;
        dongles[i].can_reset = 0;
        dongles[i].scheduler.counter = 0;
        dongles[i].coders_passed = 0;
    
        initiate_mutex(&dongles[i].reset_mtx, &sim);
        initiate_mutex(&dongles[i].dongle_mtx, &sim);
        initiate_mutex(&dongles[i].passed_mtx, &sim);
        initiate_mutex(&dongles[i].used_time_mtx, &sim);
        initiate_mutex(&dongles[i].scheduler.counter_mtx, &sim);
    }


    for (int i = 0; i < size; i++)
    {
        coders[i].coder_id = i + 1;
        coders[i].compile_count = 0;
        coders[i].last_compile_time = get_time_us();
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

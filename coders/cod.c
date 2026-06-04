/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cod.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/05/18 20:28:15 by brouane          ###   ########.fr       */
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

void set_coders_passed(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->passed_mtx);
    dongle->coders_passed += 1;
    pthread_mutex_unlock(&dongle->passed_mtx);
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

int get_coders_passed(t_dongle *dongle)
{
    int answer;
    pthread_mutex_lock(&dongle->passed_mtx);
    answer = dongle->coders_passed;
    pthread_mutex_unlock(&dongle->passed_mtx);
    return answer;
}

void reset_passed(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->passed_mtx);
    dongle->coders_passed -= 1;
    pthread_mutex_unlock(&dongle->passed_mtx);
}

static void wait_dongle_ready(t_dongle *d, t_simulation *sim)
{
    long long cooldown = ms_to_us(sim->args.dongle_cooldown);

    while (!dongle_is_ready(d, cooldown))
        precise_sleep(1, sim);
}

static long long compute_deadline(t_coder *coder, t_simulation *sim)
{
    return get_last_compile_time(coder) + ms_to_us(sim->args.time_to_burnout) - sim->start_time;
}

static void edf_register(t_dongle *d, long long deadline)
{
    pthread_mutex_lock(&d->scheduler.counter_mtx);
    d->scheduler.order[d->scheduler.counter++] = deadline;
    pthread_mutex_unlock(&d->scheduler.counter_mtx);
}

static void edf_wait_turn(t_dongle *d, long long my_deadline, t_code_sim *code_sim)
{
    while (1)
    {
        short counter = get_counter(d);
        short passed  = get_coders_passed(d);

        if (counter == 0 || (counter == 1 && passed == 1))
            return;

        if (counter == 2 && passed == 2)
        {
            pthread_mutex_lock(&d->scheduler.counter_mtx);
            long long a = d->scheduler.order[0];
            long long b = d->scheduler.order[1];
            pthread_mutex_unlock(&d->scheduler.counter_mtx);
            
            long long winner = (a < b) ? a : b;
            
            long long timestamp = get_time_ms() - us_to_ms(code_sim->sim->start_time);
            printf("at %lld coder %d with %lld | [0] %lld [1] %lld, winner %lld\n", timestamp, code_sim->coder->coder_id, my_deadline, a, b, winner);

            if (winner == my_deadline)
                return;
        }

        precise_sleep(1, code_sim->sim);
    }
}

static void edf_reset(t_dongle *d)
{
    pthread_mutex_lock(&d->scheduler.counter_mtx);
    d->scheduler.counter = 0;
    pthread_mutex_unlock(&d->scheduler.counter_mtx);
}

static int dongle_is_ready(t_dongle *d, long long cooldown)
{
    long long now = get_time_us();
    long long elapsed = now - get_last_used_time(d);

    return (elapsed >= cooldown);
}

static void take_dongle(t_code_sim *cs, t_dongle *d)
{
    t_simulation *sim = cs->sim;
    t_coder *coder = cs->coder;

    wait_dongle_ready(d, sim);

    if (sim->is_edf)
    {
        long long deadline = compute_deadline(coder, sim);
        edf_register(d, deadline);
        edf_wait_turn(d, deadline, cs);
    }

    while (1)
    {
        pthread_mutex_lock(&d->dongle_mtx);

        if (dongle_is_ready(d, ms_to_us(sim->args.dongle_cooldown)))
            break;

        pthread_mutex_unlock(&d->dongle_mtx);
        precise_sleep(1, sim);
    }

    edf_reset(d);
    reset_passed(d);

    log_action(sim, coder, "has taken a dongle");
}

void compile(t_code_sim *cs)
{
    set_coders_passed(cs->coder->first_dongle);
    log_action(cs->sim, cs->coder, "has tried to take first dongle");
    take_dongle(cs, cs->coder->first_dongle);

    set_coders_passed(cs->coder->second_dongle);
    log_action(cs->sim, cs->coder, "has tried to take second dongle");
    take_dongle(cs, cs->coder->second_dongle);

    log_action(cs->sim, cs->coder, "is compiling");
    precise_sleep(cs->sim->args.time_to_compile, cs->sim);

    long long now = get_time_us();
    set_last_compile_time(cs->coder, now);
    set_last_used_time(cs->coder->first_dongle, now);
    set_last_used_time(cs->coder->second_dongle, now);

    pthread_mutex_unlock(&cs->coder->first_dongle->dongle_mtx);
    log_action(cs->sim, cs->coder, "has dropped first dongle");

    pthread_mutex_unlock(&cs->coder->second_dongle->dongle_mtx);
    log_action(cs->sim, cs->coder, "has dropped second dongle");
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
        // dongles[i].is_available = 1;
        // dongles[i].can_reset = 0;
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 22:37:10 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long get_last_compile_time(t_coder *coder, t_simulation *sim)// good
{
    lock_mutex(&coder->state_mtx, sim);
    long long answer = coder->last_compile_time;
    unlock_mutex(&coder->state_mtx, sim);
    return answer;
}

long long get_compile_count(t_coder *coder, t_simulation *sim)// good
{
    lock_mutex(&coder->state_mtx, sim);
    long long answer = coder->compile_count;
    unlock_mutex(&coder->state_mtx, sim);
    return answer;
}

long long get_last_used_time(t_dongle *dongle, t_simulation *sim)// good
{
    lock_mutex(&dongle->used_time_mtx, sim);
    long long answer = dongle->last_used_time;
    unlock_mutex(&dongle->used_time_mtx, sim);
    return answer;
}

long long get_start_time(t_simulation *sim)// good
{
    lock_mutex(&sim->start_time_mtx, sim);
    long long answer = sim->start_time;
    unlock_mutex(&sim->start_time_mtx, sim);
    return answer;
}

void set_last_compile_time(t_coder *coder, long long now, t_simulation *sim)// good
{
    lock_mutex(&coder->state_mtx, sim);
    coder->last_compile_time = now;
    unlock_mutex(&coder->state_mtx, sim);
}

void set_compile_count(t_coder *coder, t_simulation *sim)// good
{
    lock_mutex(&coder->state_mtx, sim);
    coder->compile_count++;
    unlock_mutex(&coder->state_mtx, sim);
}

void set_last_used_time(t_dongle *dongle, long long time, t_simulation *sim)// good
{
    lock_mutex(&dongle->used_time_mtx, sim);
    dongle->last_used_time = time;
    unlock_mutex(&dongle->used_time_mtx, sim);
}

void set_coders_passed(t_dongle *dongle, t_simulation *sim)// good
{
    lock_mutex(&dongle->passed_mtx, sim);
    dongle->coders_passed += 1;
    unlock_mutex(&dongle->passed_mtx, sim);
}

void set_finished(t_simulation *sim)
{
    lock_mutex(&sim->is_finished_mtx, sim);
    sim->is_finished = 1;
    unlock_mutex(&sim->is_finished_mtx, sim);
}

short is_finished(t_simulation *sim)
{
    lock_mutex(&sim->is_finished_mtx, sim);
    short answer = sim->is_finished;
    unlock_mutex(&sim->is_finished_mtx, sim);
    return answer;
}

short get_ready(t_simulation *sim)// good
{
    lock_mutex(&sim->is_ready_mtx, sim);
    short answer = sim->is_all_ready;
    unlock_mutex(&sim->is_ready_mtx, sim);
    return answer;
}

void sync_threads(t_simulation *sim)// good
{
    while (!get_ready(sim))
        usleep(1000);
}

int get_counter(t_dongle *dongle, t_simulation *sim)// good
{
    int answer;
    lock_mutex(&dongle->scheduler.counter_mtx, sim);
    answer = dongle->scheduler.counter;
    unlock_mutex(&dongle->scheduler.counter_mtx, sim);
    return answer;
}

int get_coders_passed(t_dongle *dongle, t_simulation *sim)// good
{
    int answer;
    lock_mutex(&dongle->passed_mtx, sim);
    answer = dongle->coders_passed;
    unlock_mutex(&dongle->passed_mtx, sim);
    return answer;
}

// void get_order(t_code_sim *code_sim)
// {
//     pthread_mutex_lock(&code_sim->coder->first_dongle->scheduler.counter_mtx);

//     short *counter;
//     long long *order;

//     counter = &code_sim->coder->first_dongle->scheduler.counter;
//     order   = code_sim->coder->first_dongle->scheduler.order;

//     order[*counter] = code_sim->coder->coder_id;
//     (*counter)++;

//     printf("[get_order] Coder %d assigned order %d on dongle %d\n",
//        code_sim->coder->coder_id,
//        *counter,
//        code_sim->coder->first_dongle->dongle_id);
    
//     pthread_mutex_unlock(&code_sim->coder->first_dongle->scheduler.counter_mtx);
// }

// void reset_order(t_scheduler *scheduler)
// {
//     pthread_mutex_unlock(&scheduler->counter_mtx);
//     scheduler->order[0] = 0;
//     scheduler->order[1] = 0;
//     scheduler->counter = 0;
//     pthread_mutex_unlock(&scheduler->counter_mtx);

// }

void reset_passed(t_dongle *dongle, t_simulation *sim)// good
{
    lock_mutex(&dongle->passed_mtx, sim);
    dongle->coders_passed -= 1;
    unlock_mutex(&dongle->passed_mtx, sim);
}

void wait_dongle_ready(t_dongle *d, t_simulation *sim)
{
    long long cooldown = ms_to_us(sim->args.dongle_cooldown);

    while (!dongle_is_ready(d, cooldown, sim))
    {
        if (is_finished(sim))
            return;
        usleep(1000);
    }
}

long long compute_deadline(t_coder *coder, t_simulation *sim)// good
{
    return get_last_compile_time(coder, sim) + ms_to_us(sim->args.time_to_burnout) - get_start_time(sim);
}

void edf_register(t_dongle *d, long long deadline, t_simulation *sim)// good
{
    lock_mutex(&d->scheduler.counter_mtx, sim);
    d->scheduler.order[d->scheduler.counter++] = deadline;
    unlock_mutex(&d->scheduler.counter_mtx, sim);
}

void edf_wait_turn(t_dongle *d, long long my_deadline, t_code_sim *code_sim)
{
    while (1)
    {
        if (is_finished(code_sim->sim))
            return;

        short counter = get_counter(d, code_sim->sim);
        short passed  = get_coders_passed(d, code_sim->sim);

        if (counter == 0 || (counter == 1 && passed == 1))
            return;

        if (counter == 2 && passed == 2)
        {
            lock_mutex(&d->scheduler.counter_mtx, code_sim->sim);
            long long a = d->scheduler.order[0];
            long long b = d->scheduler.order[1];
            unlock_mutex(&d->scheduler.counter_mtx, code_sim->sim);
            
            long long winner = (a < b) ? a : b;
            if (winner == my_deadline)
                return;
        }
		usleep(1000);
    }
}

void edf_reset(t_dongle *d, t_simulation *sim)// good
{
    lock_mutex(&d->scheduler.counter_mtx, sim);
    d->scheduler.counter = 0;
    d->scheduler.order[0] = 0;
    d->scheduler.order[1] = 0;
    unlock_mutex(&d->scheduler.counter_mtx, sim);
}

int dongle_is_ready(t_dongle *d, long long cooldown, t_simulation *sim)// good
{
    long long now = get_time_us();

    long long elapsed = now - get_last_used_time(d, sim);

    return (elapsed >= cooldown);
}

void take_dongle(t_code_sim *cs, t_dongle *d)
{
    t_simulation *sim = cs->sim;
    t_coder *coder = cs->coder;

    wait_dongle_ready(d, sim);
    if (is_finished(sim))
        return;

    if (sim->is_edf)
    {
        long long deadline = compute_deadline(coder, sim);
        edf_register(d, deadline, sim);
        edf_wait_turn(d, deadline, cs);
    }

    if (is_finished(sim))
        return;

    while (1)
    {
        lock_mutex(&d->dongle_mtx, sim);

        if (is_finished(sim))
        {
            unlock_mutex(&d->dongle_mtx, sim);
            return;
        }

        if (dongle_is_ready(d, ms_to_us(sim->args.dongle_cooldown), sim))
            break;

        unlock_mutex(&d->dongle_mtx, sim);
        usleep(1000);
    }

    edf_reset(d, sim);
    reset_passed(d, sim);
    log_action(sim, coder, "has taken a dongle");
}

void compile(t_code_sim *cs)// good
{
    set_coders_passed(cs->coder->first_dongle, cs->sim);
    log_action(cs->sim, cs->coder, "has tried to take first dongle");
    take_dongle(cs, cs->coder->first_dongle);
    if (is_finished(cs->sim))
        return;

    set_coders_passed(cs->coder->second_dongle, cs->sim);
    log_action(cs->sim, cs->coder, "has tried to take second dongle");
    take_dongle(cs, cs->coder->second_dongle);
    if (is_finished(cs->sim))
    {
        unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
        return;
    }

    log_action(cs->sim, cs->coder, "is compiling");

    long long now = get_time_us();
    set_last_compile_time(cs->coder, now, cs->sim);

    precise_sleep(cs->sim->args.time_to_compile, cs->sim);

    set_last_used_time(cs->coder->first_dongle, now, cs->sim);
    set_last_used_time(cs->coder->second_dongle, now, cs->sim);

    unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
    log_action(cs->sim, cs->coder, "has dropped first dongle");

    unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
    log_action(cs->sim, cs->coder, "has dropped second dongle");
}

void	debug(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is debugging");
	precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);
}

void	refactor(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is refactoring");
	precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
	if (!is_finished(code_sim->sim))
		set_compile_count(code_sim->coder, code_sim->sim);
}

void *main_loop(void *arg)// good
{
    t_code_sim *code_sim = (t_code_sim *)arg;

    sync_threads(code_sim->sim);

    long long required = code_sim->sim->args.number_of_compiles_required;

    while (!is_finished(code_sim->sim))
    {
        long long compile_count = get_compile_count(code_sim->coder, code_sim->sim);

        if (compile_count == required)
            break;
        
        compile(code_sim);
        debug(code_sim);
        refactor(code_sim);
    }
    return NULL;
}

short check_if_coder_burned_out(t_simulation *sim)// good
{

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        if (get_compile_count(&sim->coders[i], sim)
            >= sim->args.number_of_compiles_required)
            continue;

        long long last_compile_time = get_last_compile_time(&sim->coders[i], sim);
        long long now = get_time_us();

        if (now - last_compile_time >= ms_to_us(sim->args.time_to_burnout))
        {
            log_action(sim, &sim->coders[i], "burned out");
            return 1;
        }
    }
    return 0;
}

void check_if_all_compiles_done(t_simulation *sim)// good
{
    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        long long compile_count = get_compile_count(&sim->coders[i], sim);

        if (compile_count != sim->args.number_of_compiles_required)
            return;
    }
    set_finished(sim);
}

void *the_watcher(void *arg)// good
{
    t_simulation *sim = (t_simulation *)arg;

    sync_threads(sim);

    while (!is_finished(sim))
    {
        if (check_if_coder_burned_out(sim))
        {
            set_finished(sim);
            return NULL;
        }
        
        if (!is_finished(sim))
        {
            check_if_all_compiles_done(sim);
            usleep(100);
        }
    }
    
    return NULL;
}

void program_starter(t_simulation *sim)// good
{
    int num_of_coders = sim->args.number_of_coders;
    t_code_sim *codes_sims = malloc(sizeof(t_code_sim) * num_of_coders);

    sim->codes_sims = codes_sims;

    if (!codes_sims)
        freedom(sim, 1);

    for (int i = 0; i < num_of_coders; i++)
    {
        codes_sims[i].sim = sim;
        codes_sims[i].coder = &sim->coders[i];

        thread_create(&sim->coders[i].coder, main_loop, &codes_sims[i]);
    }
    watcher_thread_create(&sim->watcher_thread, the_watcher, sim);

    lock_mutex(&sim->start_time_mtx, sim);
    sim->start_time = get_time_us();
    unlock_mutex(&sim->start_time_mtx, sim);

    for (int i = 0; i < num_of_coders; i++)
    {
        set_last_compile_time(codes_sims[i].coder, get_start_time(codes_sims[i].sim), codes_sims[i].sim);
    }

    lock_mutex(&sim->is_ready_mtx, sim);
    sim->is_all_ready = 1;
    unlock_mutex(&sim->is_ready_mtx, sim);

    for (int i = 0; i < num_of_coders; i++)
        thread_join(&sim->coders[i].coder, sim);
    thread_join(&sim->watcher_thread, sim);
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

    t_simulation sim;

    sim.coders = coders;
    sim.dongles = dongles;

    if (!coders || !dongles)
        freedom(&sim, 0);

    sim.args = data;
    sim.is_finished = 0;
    sim.is_all_ready = 0;
    sim.is_edf = 0;
    if (strcmp(sim.args.scheduler , "edf") == 0)
        sim.is_edf = 1;

    initiate_mutex(&sim.log_mtx, &sim);
    initiate_mutex(&sim.start_time_mtx, &sim);
    initiate_mutex(&sim.is_ready_mtx, &sim);
    initiate_mutex(&sim.is_finished_mtx, &sim);

    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i + 1;
        dongles[i].last_used_time = 0;
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

    freedom(&sim, 1);

    return 0;
}

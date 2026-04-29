/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 11:18:01 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

unsigned short set_finished(t_simulation *sim)
{
    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_lock(&sim->coders[i].state_mtx);
        unsigned long long count = sim->coders[i].compile_count;
        pthread_mutex_unlock(&sim->coders[i].state_mtx);

        if (count != sim->args.number_of_compiles_required)
            return 0;
    }

    pthread_mutex_lock(&sim->is_finished_mtx);
    sim->is_finished = 1;
    pthread_mutex_unlock(&sim->is_finished_mtx);
    return sim->is_finished;
}

// pthread_mutex_t d;

unsigned short is_finished(t_simulation *sim)
{
    // pthread_mutex_init(&d, NULL);
    // pthread_mutex_lock(&d);
    unsigned short answer = set_finished(sim);
    // pthread_mutex_unlock(&d);

    
    return answer;
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

static void compile(t_code_sim *code_sim)
{
    pthread_mutex_lock(&code_sim->coder->first_dongle->mtx);
    log_action(code_sim, "has taken a first dongle");
    pthread_mutex_lock(&code_sim->coder->second_dongle->mtx);
    log_action(code_sim, "has taken a second dongle");
    
    log_action(code_sim, "is compiling");
    precise_sleep(code_sim->sim->args.time_to_compile, code_sim->sim);
    
    pthread_mutex_lock(&code_sim->coder->state_mtx);
    code_sim->coder->compile_count++;
    pthread_mutex_unlock(&code_sim->coder->state_mtx);
    
    pthread_mutex_unlock(&code_sim->coder->first_dongle->mtx);
    pthread_mutex_unlock(&code_sim->coder->second_dongle->mtx);
}

static void debug(t_code_sim *code_sim)
{ 
    log_action(code_sim, "is debugging");
    precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);

}

static void refactor(t_code_sim *code_sim)
{ 
    log_action(code_sim, "is refactoring");
    precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
}

void *main_loop(void *arg)
{
    t_code_sim *code_sim = (t_code_sim *)arg;

    sync_threads(code_sim->sim);

    unsigned long long required = code_sim->sim->args.number_of_compiles_required;
    while (!is_finished(code_sim->sim))
    {
        if (code_sim->coder->compile_count == required)
            break;

        compile(code_sim);
        debug(code_sim);
        refactor(code_sim);

    }
    free(code_sim);
    return NULL;
}

void program_starter(t_simulation *sim)
{
    // unsigned long long start_time = get_time_ms();
    printf("program_starter\n");

    int num_of_coders = sim->args.number_of_coders;


    if (num_of_coders == 0)
    {
        return ;
    }
    // else if (num_of_coders == 1)
    //     ;
    else
    {

        for (int i = 0; i < num_of_coders; i++)
        {
            // pthread_t *coder_thr = &sim->coders[i].coder;

            // t_code_sim *code_sim;
            // code_sim->coder = coder_thr;
            // code_sim->sim = sim;
            t_code_sim *code_sim = malloc(sizeof(t_code_sim));
            code_sim->sim = sim;
            code_sim->coder = &sim->coders[i];

            pthread_create(&sim->coders[i].coder, NULL, main_loop, code_sim);
        }
    }

    pthread_mutex_lock(&sim->is_ready_mtx);
    sim->is_all_ready = 1;
    sim->start_time = get_time_ms();
    pthread_mutex_unlock(&sim->is_ready_mtx);
    // printf("ttttttttttttt %lld\n", sim->start_time - start_time);

    for (int i = 0; i < num_of_coders; i++)
    {
        pthread_join(sim->coders[i].coder, NULL);
    }
}

void t(t_simulation *sim)
{
    sim->args.number_of_coders = 666;
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

    sim.args = data;
    sim.coders = coders;
    sim.dongles = dongles;
    sim.is_finished = 0;
    sim.is_all_ready = 0;

    pthread_mutex_init(&sim.log_mtx, NULL);
    pthread_mutex_init(&sim.is_ready_mtx, NULL);
    pthread_mutex_init(&sim.is_finished_mtx, NULL);

    for (int i = 0; i < size; i++)
    {
        dongles[i].dongle_id = i + 1;
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
        pthread_mutex_init(&coders[i].state_mtx, NULL);

        if (coders[i].coder_id % 2 == 0)
        {
            // printf("ggg %d\n", coders[i].coder_id);
            coders[i].first_dongle = &dongles[i];
            coders[i].second_dongle = &dongles[(i + 1) % size];
        }
        else
        {
            // printf("ddd %d\n", coders[i].coder_id);
            coders[i].first_dongle = &dongles[(i + 1) % size];
            coders[i].second_dongle = &dongles[i];
        }
        
    }

    for (int i = 0; i < size; i++)
    {
        // printf("%d %d %d\n", coders[i].coder_id, coders[i].first_dongle->dongle_id, coders[i].second_dongle->dongle_id);
    }
    program_starter(&sim);

    free(coders);
    free(dongles);
    return 0;
}

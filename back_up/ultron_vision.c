/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ultron_vision.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:28 by brouane           #+#    #+#             */
/*   Updated: 2026/06/06 15:52:39 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void initiate_mutex(pthread_mutex_t *mutex, t_simulation *sim)// good
{
    int result = pthread_mutex_init(mutex, NULL);

    if (result != 0)
    {
        lock_mutex(&sim->log_mtx, sim);
        printf("Error: %s\n", strerror(result));
        unlock_mutex(&sim->log_mtx, sim);
        freedom(sim, 1);
    }
}

void destroy_them_all(t_simulation *sim)// good
{
    pthread_mutex_destroy(&sim->log_mtx);
    pthread_mutex_destroy(&sim->is_ready_mtx);
    pthread_mutex_destroy(&sim->start_time_mtx);
    pthread_mutex_destroy(&sim->is_finished_mtx);

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_destroy(&sim->dongles[i].reset_mtx);
        pthread_mutex_destroy(&sim->dongles[i].dongle_mtx);
        pthread_mutex_destroy(&sim->dongles[i].passed_mtx);
        pthread_mutex_destroy(&sim->dongles[i].used_time_mtx);
        pthread_mutex_destroy(&sim->dongles[i].scheduler.counter_mtx);
        pthread_mutex_destroy(&sim->coders[i].state_mtx);
    }
}

void lock_mutex(pthread_mutex_t *mutex, t_simulation *sim)// good
{
    int result = pthread_mutex_lock(mutex);
    if (result != 0)
    {
        lock_mutex(&sim->log_mtx, sim);
        printf("Error: %s\n", strerror(result));
        unlock_mutex(&sim->log_mtx, sim);
        freedom(sim, 1);
    }
}

void unlock_mutex(pthread_mutex_t *mutex, t_simulation *sim)// good
{
    int result = pthread_mutex_unlock(mutex);
    if (result != 0)
    {
        lock_mutex(&sim->log_mtx, sim);
        printf("Error: %s\n", strerror(result));
        unlock_mutex(&sim->log_mtx, sim);
        freedom(sim, 1);
    }
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ultron_vision.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:28 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 23:20:46 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void initiate_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
    int result = pthread_mutex_init(mutex, NULL);

    if (result != 0)
    {
        printf("Error: %s\n", strerror(result));
        freedom(sim->coders, sim->dongles);
        exit(1);
    }
}

void destroy_them_all(t_simulation *sim)
{
    pthread_mutex_destroy(&sim->log_mtx);
    pthread_mutex_destroy(&sim->is_ready_mtx);
    pthread_mutex_destroy(&sim->is_finished_mtx);

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_destroy(&sim->dongles[i].mtx);
        pthread_mutex_destroy(&sim->coders[i].state_mtx);
    }
}

void lock_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
    int result = pthread_mutex_lock(mutex);
    if (result != 0)
    {
        printf("s Error: %s\n", strerror(result));
        freedom(sim->coders, sim->dongles);
        exit(1);
    }
}

void unlock_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
    int result = pthread_mutex_unlock(mutex);
    if (result != 0)
    {
        printf("Error: %s\n", strerror(result));
        freedom(sim->coders, sim->dongles);
        exit(1);
    }
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mutex_manager.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:28 by brouane           #+#    #+#             */
/*   Updated: 2026/06/15 18:34:42 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <string.h>
#include <errno.h>

void	initiate_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
	int	result;

	result = pthread_mutex_init(mutex, NULL);
	if (result != 0)
	{
		lock_mutex(&sim->log_mtx, sim);
		fprintf(stderr, "Error: mutex init: %s\n", strerror(result));
		unlock_mutex(&sim->log_mtx, sim);
		freedom(sim, 0);
	}
}

void	lock_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
	int	result;

	result = pthread_mutex_lock(mutex);
	if (result != 0)
	{
		lock_mutex(&sim->log_mtx, sim);
		fprintf(stderr, "Error: mutex lock: %s\n", strerror(result));
		unlock_mutex(&sim->log_mtx, sim);
		freedom(sim, 0);
	}
}

void	unlock_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
	int	result;

	result = pthread_mutex_unlock(mutex);
	if (result != 0)
	{
		lock_mutex(&sim->log_mtx, sim);
		fprintf(stderr, "Error: mutex unlock: %s\n", strerror(result));
		unlock_mutex(&sim->log_mtx, sim);
		freedom(sim, 0);
	}
}

void	destroy_them_all(t_simulation *sim)
{
	int	i;

	pthread_mutex_destroy(&sim->log_mtx);
	pthread_mutex_destroy(&sim->is_ready_mtx);
	pthread_mutex_destroy(&sim->start_time_mtx);
	pthread_mutex_destroy(&sim->is_finished_mtx);
	i = 0;
	while (i < sim->args.number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].dongle_mtx);
		pthread_mutex_destroy(&sim->dongles[i].used_time_mtx);
		pthread_mutex_destroy(&sim->dongles[i].scheduler.order_mtx);
		pthread_mutex_destroy(&sim->coders[i].state_mtx);
		i++;
	}
}

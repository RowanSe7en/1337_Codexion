/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kang.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 20:50:03 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 19:49:20 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	thread_create(pthread_t *coder, void *func, t_code_sim *code_sim)
{
	int	result;

	result = pthread_create(coder, NULL, func, code_sim);
	if (result != 0)
	{
		lock_mutex(&code_sim->sim->log_mtx, code_sim->sim);
		printf("Error: %s\n", strerror(result));
		unlock_mutex(&code_sim->sim->log_mtx, code_sim->sim);
		freedom(code_sim->sim, 1);
	}
}

void	watcher_thread_create(pthread_t *wt, void *func, t_simulation *sim)
{
	int	result;

	result = pthread_create(wt, NULL, func, sim);
	if (result != 0)
	{
		lock_mutex(&sim->log_mtx, sim);
		printf("Error: %s\n", strerror(result));
		unlock_mutex(&sim->log_mtx, sim);
		freedom(sim, 1);
	}
}

void	thread_join(pthread_t *thread, t_simulation *sim)
{
	int	result;

	result = pthread_join(*thread, NULL);
	if (result != 0)
	{
		lock_mutex(&sim->log_mtx, sim);
		printf("Error: %s\n", strerror(result));
		unlock_mutex(&sim->log_mtx, sim);
		freedom(sim, 1);
	}
}

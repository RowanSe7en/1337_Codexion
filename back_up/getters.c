/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getters.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 22:03:11 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_last_compile_time(t_coder *coder, t_simulation *sim)
{
	long long	answer;

	lock_mutex(&coder->state_mtx, sim);
	answer = coder->last_compile_time;
	unlock_mutex(&coder->state_mtx, sim);
	return (answer);
}

long long	get_compile_count(t_coder *coder, t_simulation *sim)
{
	long long	answer;

	lock_mutex(&coder->state_mtx, sim);
	answer = coder->compile_count;
	unlock_mutex(&coder->state_mtx, sim);
	return (answer);
}

long long	get_last_used_time(t_dongle *dongle, t_simulation *sim)
{
	long long	answer;

	lock_mutex(&dongle->used_time_mtx, sim);
	answer = dongle->last_used_time;
	unlock_mutex(&dongle->used_time_mtx, sim);
	return (answer);
}

long long	get_start_time(t_simulation *sim)
{
	long long	answer;

	lock_mutex(&sim->start_time_mtx, sim);
	answer = sim->start_time;
	unlock_mutex(&sim->start_time_mtx, sim);
	return (answer);
}

int	get_counter(t_dongle *dongle, t_simulation *sim)
{
	int	answer;

	lock_mutex(&dongle->scheduler.counter_mtx, sim);
	answer = dongle->scheduler.counter;
	unlock_mutex(&dongle->scheduler.counter_mtx, sim);
	return (answer);
}

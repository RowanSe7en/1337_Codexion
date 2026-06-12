/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 19:49:20 by brouane          ###   ########.fr       */
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

void	set_last_compile_time(t_coder *coder, long long now, t_simulation *sim)
{
	lock_mutex(&coder->state_mtx, sim);
	coder->last_compile_time = now;
	unlock_mutex(&coder->state_mtx, sim);
}

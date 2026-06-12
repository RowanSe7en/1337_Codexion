/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   watcher.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 19:49:20 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	set_compile_count(t_coder *coder, t_simulation *sim)
{
	lock_mutex(&coder->state_mtx, sim);
	coder->compile_count++;
	unlock_mutex(&coder->state_mtx, sim);
}

void	set_last_used_time(t_dongle *dongle, long long time, t_simulation *sim)
{
	lock_mutex(&dongle->used_time_mtx, sim);
	dongle->last_used_time = time;
	unlock_mutex(&dongle->used_time_mtx, sim);
}

void	set_finished(t_simulation *sim)
{
	lock_mutex(&sim->is_finished_mtx, sim);
	sim->is_finished = 1;
	unlock_mutex(&sim->is_finished_mtx, sim);
}

short	is_finished(t_simulation *sim)
{
	short	answer;

	lock_mutex(&sim->is_finished_mtx, sim);
	answer = sim->is_finished;
	unlock_mutex(&sim->is_finished_mtx, sim);
	return (answer);
}

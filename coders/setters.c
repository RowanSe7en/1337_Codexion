/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setters.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 19:11:43 by brouane           #+#    #+#             */
/*   Updated: 2026/06/12 19:15:14 by brouane          ###   ########.fr       */
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

void	set_last_compile_time(t_coder *coder, long long now, t_simulation *sim)
{
	lock_mutex(&coder->state_mtx, sim);
	coder->last_compile_time = now;
	unlock_mutex(&coder->state_mtx, sim);
}

void	set_coder_times_and_ready(t_simulation *sim, int num)
{
	int	i;

	i = 0;
	while (i < num)
	{
		set_last_compile_time(sim->codes_sims[i].coder,
			get_start_time(sim->codes_sims[i].sim),
			sim->codes_sims[i].sim);
		i++;
	}
	lock_mutex(&sim->is_ready_mtx, sim);
	sim->is_all_ready = 1;
	unlock_mutex(&sim->is_ready_mtx, sim);
}

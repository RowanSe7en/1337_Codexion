/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 22:28:40 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

short	is_finished(t_simulation *sim)
{
	short	answer;

	lock_mutex(&sim->is_finished_mtx, sim);
	answer = sim->is_finished;
	unlock_mutex(&sim->is_finished_mtx, sim);
	return (answer);
}

void	sync_threads(t_simulation *sim)
{
	while (!get_ready(sim))
		usleep(1000);
}

void	reset_passed(t_dongle *dongle, t_simulation *sim)
{
	lock_mutex(&dongle->passed_mtx, sim);
	dongle->coders_passed -= 1;
	unlock_mutex(&dongle->passed_mtx, sim);
}

int	dongle_is_ready(t_dongle *d, long long cooldown, t_simulation *sim)
{
	long long	now;
	long long	elapsed;

	now = get_time_us();
	elapsed = now - get_last_used_time(d, sim);
	return (elapsed >= cooldown);
}

void	wait_dongle_ready(t_dongle *d, t_simulation *sim)
{
	long long	cooldown;

	cooldown = ms_to_us(sim->args.dongle_cooldown);
	while (!dongle_is_ready(d, cooldown, sim))
	{
		if (is_finished(sim))
			return ;
		precise_sleep(1, sim);
	}
}

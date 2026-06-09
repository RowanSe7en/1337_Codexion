/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 22:22:28 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wait_dongle_ready(t_dongle *d, t_simulation *sim)
{
	long long	cooldown;

	cooldown = ms_to_us(sim->args.dongle_cooldown);
	while (!dongle_is_ready(d, cooldown, sim))
	{
		if (is_finished(sim))
			return ;
		usleep(1000);
	}
}

long long	compute_deadline(t_coder *coder, t_simulation *sim)
{
	return (get_last_compile_time(coder, sim)
		+ ms_to_us(sim->args.time_to_burnout) - get_start_time(sim));
}

void	edf_register(t_dongle *d, long long deadline, t_simulation *sim)
{
	lock_mutex(&d->scheduler.counter_mtx, sim);
	d->scheduler.order[d->scheduler.counter++] = deadline;
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

void	edf_reset(t_dongle *d, t_simulation *sim)
{
	lock_mutex(&d->scheduler.counter_mtx, sim);
	d->scheduler.counter = 0;
	d->scheduler.order[0] = 0;
	d->scheduler.order[1] = 0;
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

int	dongle_is_ready(t_dongle *d, long long cooldown, t_simulation *sim)
{
	long long	now;
	long long	elapsed;

	now = get_time_us();
	elapsed = now - get_last_used_time(d, sim);
	return (elapsed >= cooldown);
}

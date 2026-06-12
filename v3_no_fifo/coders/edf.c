/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 19:49:20 by brouane          ###   ########.fr       */
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
		precise_sleep(1, sim);
	}
}

long long	compute_deadline(t_coder *coder, t_simulation *sim)
{
	return (get_last_compile_time(coder, sim)
		+ ms_to_us(sim->args.time_to_burnout) - get_start_time(sim));
}

void	edf_register(t_dongle *d, long long deadline, t_simulation *sim)
{
	int	i;

	lock_mutex(&d->scheduler.counter_mtx, sim);
	i = 0;
	while (i < 2)
	{
		if (d->scheduler.order[i] == 0)
		{
			d->scheduler.order[i] = deadline;
			d->scheduler.counter++;
			break ;
		}
		i++;
	}
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

void	edf_deregister(t_dongle *d, long long deadline, t_simulation *sim)
{
	int	i;

	lock_mutex(&d->scheduler.counter_mtx, sim);
	i = 0;
	while (i < 2)
	{
		if (d->scheduler.order[i] == deadline)
		{
			d->scheduler.order[i] = 0;
			d->scheduler.counter--;
			break ;
		}
		i++;
	}
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

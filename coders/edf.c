/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/15 18:15:23 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	compute_deadline(t_coder *coder, t_simulation *sim)
{
	return (get_last_compile_time(coder, sim)
		+ ms_to_us(sim->args.time_to_burnout) - get_start_time(sim));
}

void	edf_register(t_dongle *d, long long deadline,
			int coder_id, t_simulation *sim)
{
	int	i;

	lock_mutex(&d->scheduler.order_mtx, sim);
	i = 0;
	while (i < 2)
	{
		if (d->scheduler.deadlines[i] == 0)
		{
			d->scheduler.deadlines[i] = deadline;
			d->scheduler.coder_ids[i] = coder_id;
			break ;
		}
		i++;
	}
	unlock_mutex(&d->scheduler.order_mtx, sim);
}

void	edf_deregister(t_dongle *d, long long deadline,
			int coder_id, t_simulation *sim)
{
	int	i;

	lock_mutex(&d->scheduler.order_mtx, sim);
	i = 0;
	while (i < 2)
	{
		if (d->scheduler.deadlines[i] == deadline
			&& d->scheduler.coder_ids[i] == coder_id)
		{
			d->scheduler.deadlines[i] = 0;
			d->scheduler.coder_ids[i] = 0;
			break ;
		}
		i++;
	}
	unlock_mutex(&d->scheduler.order_mtx, sim);
}

void	edf_wait_turn(t_dongle *d, long long my_deadline,
			t_code_sim *cs)
{
	usleep(1000);
	while (!is_finished(cs->sim))
	{
		if (edf_early(d, my_deadline, cs->sim))
			return ;
		usleep(500);
	}
}

int	edf_early(t_dongle *d, long long my_deadline,
			t_simulation *sim)
{
	int			i;
	long long	val;

	lock_mutex(&d->scheduler.order_mtx, sim);
	i = 0;
	while (i < 2)
	{
		val = d->scheduler.deadlines[i];
		if (val != 0 && val < my_deadline)
		{
			unlock_mutex(&d->scheduler.order_mtx, sim);
			return (0);
		}
		i++;
	}
	unlock_mutex(&d->scheduler.order_mtx, sim);
	return (1);
}

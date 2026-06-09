/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/07 17:17:53 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 15:30:26 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	fifo_register(t_dongle *d, int coder_id, t_simulation *sim)
{
	lock_mutex(&d->scheduler.counter_mtx, sim);
	d->scheduler.order[d->scheduler.counter++] = coder_id;
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

void	fifo_wait_turn(t_dongle *d, int my_id, t_code_sim *cs)
{
	long long	a;
	long long	b;

	while (1)
	{
		if (is_finished(cs->sim))
			return ;
		lock_mutex(&d->scheduler.counter_mtx, cs->sim);
		a = d->scheduler.order[0];
		b = d->scheduler.order[1];
		unlock_mutex(&d->scheduler.counter_mtx, cs->sim);
		if (a == 0 && b == 0)
			return ;
		if (b == 0)
			return ;
		if (a == my_id)
			return ;
		precise_sleep(1, cs->sim);
	}
}

void	register_dongle(t_code_sim *cs, t_dongle *d, int holding_first)
{
	long long	deadline;

	if (cs->sim->is_edf)
	{
		if (holding_first)
			deadline = -1;
		else
			deadline = compute_deadline(cs->coder, cs->sim);
		edf_register(d, deadline, cs->sim);
		edf_wait_turn(d, deadline, cs);
	}
	else
	{
		fifo_register(d, cs->coder->coder_id, cs->sim);
		fifo_wait_turn(d, cs->coder->coder_id, cs);
	}
}

int	wait_and_lock_dongle(t_dongle *d, t_simulation *sim)
{
	while (1)
	{
		lock_mutex(&d->dongle_mtx, sim);
		if (is_finished(sim))
		{
			unlock_mutex(&d->dongle_mtx, sim);
			return (0);
		}
		if (dongle_is_ready(d,
				ms_to_us(sim->args.dongle_cooldown), sim))
			break ;
		unlock_mutex(&d->dongle_mtx, sim);
		precise_sleep(1, sim);
	}
	return (1);
}

int	take_dongle(t_code_sim *cs, t_dongle *d, int already_held,
		int holding_first)
{
	t_simulation	*sim;
	t_coder			*coder;

	sim = cs->sim;
	coder = cs->coder;
	wait_dongle_ready(d, sim);
	if (is_finished(sim))
		return (0);
	if (already_held)
	{
		while (!is_finished(sim))
			precise_sleep(1, sim);
		return (0);
	}
	register_dongle(cs, d, holding_first);
	if (is_finished(sim))
		return (0);
	if (!wait_and_lock_dongle(d, sim))
		return (0);
	edf_reset(d, sim);
	reset_passed(d, sim);
	log_action(sim, coder, "has taken a dongle");
	return (1);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:13:16 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	edf_i_am_min(t_dongle *d, long long my_deadline,
				t_simulation *sim)
{
	int			i;
	int			cnt;
	long long	val;

	lock_mutex(&d->scheduler.counter_mtx, sim);
	cnt = d->scheduler.counter;
	i = 0;
	while (i < cnt)
	{
		val = d->scheduler.order[i];
		if (val != 0 && val < my_deadline)
		{
			unlock_mutex(&d->scheduler.counter_mtx, sim);
			return (0);
		}
		i++;
	}
	unlock_mutex(&d->scheduler.counter_mtx, sim);
	return (1);
}

void	edf_wait_turn(t_dongle *d, long long my_deadline, t_code_sim *cs)
{
	precise_sleep(5, cs->sim);
	while (!is_finished(cs->sim))
	{
		if (edf_i_am_min(d, my_deadline, cs->sim))
			return ;
		precise_sleep(1, cs->sim);
	}
}

static void	take_dongle_wait_loop(t_code_sim *cs, t_dongle *d)
{
	t_simulation	*sim;

	sim = cs->sim;
	while (1)
	{
		lock_mutex(&d->dongle_mtx, sim);
		if (is_finished(sim))
		{
			unlock_mutex(&d->dongle_mtx, sim);
			return ;
		}
		if (dongle_is_ready(d, ms_to_us(sim->args.dongle_cooldown), sim))
			break ;
		unlock_mutex(&d->dongle_mtx, sim);
		precise_sleep(1, sim);
	}
}

void	take_dongle(t_code_sim *cs, t_dongle *d)
{
	t_simulation	*sim;
	t_coder			*coder;
	long long		deadline;

	sim = cs->sim;
	coder = cs->coder;
	wait_dongle_ready(d, sim);
	if (is_finished(sim))
		return ;
	if (sim->is_edf)
	{
		deadline = compute_deadline(coder, sim);
		edf_register(d, deadline, sim);
		edf_wait_turn(d, deadline, cs);
	}
	if (is_finished(sim))
		return ;
	take_dongle_wait_loop(cs, d);
	if (is_finished(sim))
		return ;
	edf_deregister(d, deadline, sim);
	log_action(sim, coder, "has taken a dongle");
}

void	compile(t_code_sim *cs)
{
	long long	now;

	log_action(cs->sim, cs->coder, "has tried to take first dongle");
	take_dongle(cs, cs->coder->first_dongle);
	if (is_finished(cs->sim))
		return ;
	log_action(cs->sim, cs->coder, "has tried to take second dongle");
	take_dongle(cs, cs->coder->second_dongle);
	if (is_finished(cs->sim))
	{
		unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
		return ;
	}
	log_action(cs->sim, cs->coder, "is compiling");
	precise_sleep(cs->sim->args.time_to_compile, cs->sim);
	now = get_time_us();
	set_last_compile_time(cs->coder, now, cs->sim);
	set_last_used_time(cs->coder->first_dongle, now, cs->sim);
	set_last_used_time(cs->coder->second_dongle, now, cs->sim);
	unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
	log_action(cs->sim, cs->coder, "has dropped first dongle");
	unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
	log_action(cs->sim, cs->coder, "has dropped second dongle");
}

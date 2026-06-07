/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:41:10 by brouane          ###   ########.fr       */
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
	while (1)
	{
		if (is_finished(cs->sim))
			return ;
		short counter = get_counter(d, cs->sim);
		short passed  = get_coders_passed(d, cs->sim);
		if (counter == 0 || (counter == 1 && passed == 1))
			return ;
		if (counter == 2 && passed == 2)
		{
			lock_mutex(&d->scheduler.counter_mtx, cs->sim);
			long long first = d->scheduler.order[0];
			unlock_mutex(&d->scheduler.counter_mtx, cs->sim);
			if (first == my_id)
				return ;
		}
		precise_sleep(1, cs->sim);
	}
}

int	take_dongle(t_code_sim *cs, t_dongle *d, int already_held)
{
	t_simulation	*sim = cs->sim;
	t_coder			*coder = cs->coder;

	wait_dongle_ready(d, sim);
	if (is_finished(sim))
		return (0);
	if (already_held)
	{
		while (!is_finished(sim))
			precise_sleep(1, sim);
		return (0);
	}
	if (sim->is_edf)
	{
		long long deadline = compute_deadline(coder, sim);
		edf_register(d, deadline, sim);
		edf_wait_turn(d, deadline, cs);
	}
	else
	{
		fifo_register(d, coder->coder_id, sim);
		fifo_wait_turn(d, coder->coder_id, cs);
	}
	if (is_finished(sim))
		return (0);
	while (1)
	{
		lock_mutex(&d->dongle_mtx, sim);
		if (is_finished(sim))
		{
			unlock_mutex(&d->dongle_mtx, sim);
			return (0);
		}
		if (dongle_is_ready(d, ms_to_us(sim->args.dongle_cooldown), sim))
			break ;
		unlock_mutex(&d->dongle_mtx, sim);
		precise_sleep(1, sim);
	}
	edf_reset(d, sim);
	reset_passed(d, sim);
	log_action(sim, coder, "has taken a dongle");
	return (1);
}

void	compile(t_code_sim *cs)
{
	int	same_dongle = (cs->coder->first_dongle == cs->coder->second_dongle);

	set_coders_passed(cs->coder->first_dongle, cs->sim);
	if (!take_dongle(cs, cs->coder->first_dongle, 0))
		return ;

	set_coders_passed(cs->coder->second_dongle, cs->sim);
	if (!take_dongle(cs, cs->coder->second_dongle, same_dongle))
	{
		unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
		return ;
	}

	log_action(cs->sim, cs->coder, "is compiling");
	precise_sleep(cs->sim->args.time_to_compile, cs->sim);

	if (is_finished(cs->sim))
	{
		unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
		if (!same_dongle)
			unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
		return ;
	}

	long long now = get_time_us();
	set_last_compile_time(cs->coder, now, cs->sim);
	set_last_used_time(cs->coder->first_dongle, now, cs->sim);
	set_last_used_time(cs->coder->second_dongle, now, cs->sim);

	unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);

	if (!same_dongle)
		unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
}
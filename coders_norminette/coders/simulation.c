/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 22:50:39 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	edf_is_my_turn(t_dongle *d, long long my_deadline,
				t_simulation *sim)
{
	long long	a;
	long long	b;
	long long	winner;

	lock_mutex(&d->scheduler.counter_mtx, sim);
	a = d->scheduler.order[0];
	b = d->scheduler.order[1];
	unlock_mutex(&d->scheduler.counter_mtx, sim);
	if (a < b)
		winner = a;
	else
		winner = b;
	return (winner == my_deadline);
}

void	edf_wait_turn(t_dongle *d, long long my_deadline, t_code_sim *code_sim)
{
	short	counter;
	short	passed;

	while (1)
	{
		if (is_finished(code_sim->sim))
			return ;
		counter = get_counter(d, code_sim->sim);
		passed = get_coders_passed(d, code_sim->sim);
		if (counter == 0 || (counter == 1 && passed == 1))
			return ;
		if (counter == 2 && passed == 2
			&& edf_is_my_turn(d, my_deadline, code_sim->sim))
			return ;
		usleep(1000);
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
		usleep(1000);
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
	edf_reset(d, sim);
	reset_passed(d, sim);
	log_action(sim, coder, "has taken a dongle");
}

void	compile(t_code_sim *cs)
{
	long long	now;

	set_coders_passed(cs->coder->first_dongle, cs->sim);
	log_action(cs->sim, cs->coder, "has tried to take first dongle");
	take_dongle(cs, cs->coder->first_dongle);
	if (is_finished(cs->sim))
		return ;
	set_coders_passed(cs->coder->second_dongle, cs->sim);
	log_action(cs->sim, cs->coder, "has tried to take second dongle");
	take_dongle(cs, cs->coder->second_dongle);
	if (is_finished(cs->sim))
	{
		unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
		return ;
	}
	log_action(cs->sim, cs->coder, "is compiling");
	now = get_time_us();
	set_last_compile_time(cs->coder, now, cs->sim);
	precise_sleep(cs->sim->args.time_to_compile, cs->sim);
	now = get_time_us();
	set_last_used_time(cs->coder->first_dongle, now, cs->sim);
	set_last_used_time(cs->coder->second_dongle, now, cs->sim);
	unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
	log_action(cs->sim, cs->coder, "has dropped first dongle");
	unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
	log_action(cs->sim, cs->coder, "has dropped second dongle");
}

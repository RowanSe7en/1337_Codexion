/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/15 18:38:20 by brouane          ###   ########.fr       */
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

int	dongle_is_ready(t_dongle *d, long long cooldown, t_simulation *sim)
{
	long long	now;
	long long	elapsed;

	now = get_time_us();
	elapsed = now - get_last_used_time(d, sim);
	return (elapsed >= cooldown);
}

int	take_dongle_wait_loop(t_code_sim *cs, t_dongle *d)
{
	t_simulation	*sim;

	sim = cs->sim;
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
	return (1);
}

long long	handle_scheduler(t_code_sim *cs, t_dongle *d)
{
	t_simulation	*sim;
	t_coder			*coder;
	long long		deadline;

	sim = cs->sim;
	coder = cs->coder;
	deadline = 0;
	if (sim->is_edf)
	{
		deadline = compute_deadline(coder, sim);
		edf_register(d, deadline, coder->coder_id, sim);
		edf_wait_turn(d, deadline, cs);
	}
	else
	{
		fifo_register(d, coder->coder_id, sim);
		fifo_wait_turn(d, coder->coder_id, cs);
	}
	return (deadline);
}

int	take_dongle(t_code_sim *cs, t_dongle *d)
{
	t_simulation	*sim;
	t_coder			*coder;
	long long		deadline;
	int				acquired;

	sim = cs->sim;
	coder = cs->coder;
	wait_dongle_ready(d, sim);
	if (is_finished(sim))
		return (0);
	deadline = handle_scheduler(cs, d);
	if (is_finished(sim))
	{
		if (sim->is_edf)
			edf_deregister(d, deadline, coder->coder_id, sim);
		else
			fifo_deregister(d, coder->coder_id, sim);
		return (0);
	}
	acquired = take_dongle_wait_loop(cs, d);
	if (sim->is_edf)
		edf_deregister(d, deadline, coder->coder_id, sim);
	else
		fifo_deregister(d, coder->coder_id, sim);
	if (!acquired || is_finished(sim))
	{
		if (acquired)
			unlock_mutex(&d->dongle_mtx, sim);
		return (0);
	}
	log_action(sim, coder, "has taken a dongle", 0);
	return (1);
}

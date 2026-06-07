/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf_scheduler.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 22:28:14 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	compute_deadline(t_coder *coder, t_simulation *sim)
{
	return (get_last_compile_time(coder, sim)
		+ ms_to_us(sim->args.time_to_burnout)
		- get_start_time(sim));
}

void	edf_register(t_dongle *d, long long deadline, t_simulation *sim)
{
	lock_mutex(&d->scheduler.counter_mtx, sim);
	d->scheduler.order[d->scheduler.counter++] = deadline;
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

int	has_priority(t_dongle *d, long long my_deadline,
	t_code_sim *code_sim)
{
	long long	a;
	long long	b;

	lock_mutex(&d->scheduler.counter_mtx, code_sim->sim);
	a = d->scheduler.order[0];
	b = d->scheduler.order[1];
	unlock_mutex(&d->scheduler.counter_mtx, code_sim->sim);
	if (a < b && a == my_deadline)
		return (1);
	if (b <= a && b == my_deadline)
		return (1);
	return (0);
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
		if (counter == 2 && passed == 2)
		{
			if (has_priority(d, my_deadline, code_sim))
				return ;
		}
		precise_sleep(1, code_sim->sim);
	}
}

void	edf_reset(t_dongle *d, t_simulation *sim)
{
	lock_mutex(&d->scheduler.counter_mtx, sim);
	d->scheduler.counter = 0;
	d->scheduler.order[0] = 0;
	d->scheduler.order[1] = 0;
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

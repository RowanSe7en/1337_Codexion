/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/06 16:33:32 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	compute_deadline(t_coder *coder, t_simulation *sim)
{
	return get_last_compile_time(coder, sim)
		+ ms_to_us(sim->args.time_to_burnout)
		- get_start_time(sim);
}

void	edf_register(t_dongle *d, long long deadline, t_simulation *sim)
{
	lock_mutex(&d->scheduler.counter_mtx, sim);
	d->scheduler.order[d->scheduler.counter++] = deadline;
	unlock_mutex(&d->scheduler.counter_mtx, sim);
}

void	edf_wait_turn(t_dongle *d, long long my_deadline, t_code_sim *code_sim)
{
	while (1)
	{
		if (is_finished(code_sim->sim))
			return ;
		short counter = get_counter(d, code_sim->sim);
		short passed  = get_coders_passed(d, code_sim->sim);
		if (counter == 0 || (counter == 1 && passed == 1))
			return ;
		if (counter == 2 && passed == 2)
		{
			lock_mutex(&d->scheduler.counter_mtx, code_sim->sim);
			long long a = d->scheduler.order[0];
			long long b = d->scheduler.order[1];
			unlock_mutex(&d->scheduler.counter_mtx, code_sim->sim);
			long long winner = (a < b) ? a : b;
			if (winner == my_deadline)
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
